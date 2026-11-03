#include "Labs/Lab37_HitWindow.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab37Text
{
BH_TEXT(StaggerLog, "  프레임 %llu: 맞음 → 경직, 베기 중단", "  フレーム %llu: 被弾 → 怯み、斬りを中断");
BH_TEXT(EventLog,   "  프레임 %llu: %s → 히트박스 %s", "  フレーム %llu: %s → ヒットボックス %s");
BH_TEXT(On,         "켬", "オン");
BH_TEXT(Off,        "끔", "オフ");
BH_TEXT(Plan,       "베기 %.2f초, 히트박스 구간 %.2f~%.2f초. %.2f초에 적에게 맞아 경직", "斬り %.2f 秒、ヒットボックス区間 %.2f〜%.2f 秒。%.2f 秒に敵の攻撃を受けて怯む");
BH_TEXT(Passing,    "칼이 지나가는 중: 히트박스 켜짐", "剣が通過中: ヒットボックスがオン");
BH_TEXT(StillPlay,  "경직 1초 뒤 베기 재생 중: %s", "怯みの 1 秒後、斬りを再生中: %s");
BH_TEXT(Yes,        "예", "はい");
BH_TEXT(No,         "아니오", "いいえ");
BH_TEXT(AfterCut,   "베기가 끊긴 뒤: 히트박스 꺼짐 (가만히 서 있는 칼에 적이 닿아도 안 맞음)", "斬りが中断された後: ヒットボックスがオフ（止まっている剣に敵が触れてもダメージなし）");
}

void ABH_Knight::BeginPlay()
{
	Super::BeginPlay();

	// 에디터라면 베기 애니메이션 에셋에 노티파이를 달아 두는 일. 칼이 지나가는 구간 = 길이의 30% ~ 60%.
	// [JA] エディターなら斬りアニメーションのアセットに通知(Notify)を付けておく作業。剣が通過する区間 = 長さの 30% 〜 60%。
	SlashMontage = BugHunt::MakeMontage(BugHunt::LoadTutorialAnim(TEXT("Tutorial_Walk_Fwd")));
	const float Length = SlashMontage->GetPlayLength();
	BugHunt::AddNotify(SlashMontage, Length * 0.3f, TEXT("HitOn"));
	BugHunt::AddNotify(SlashMontage, Length * 0.6f, TEXT("HitOff"));
}

void ABH_Knight::Slash()
{
	GetAnim()->Montage_Play(SlashMontage);
}

void ABH_Knight::Stagger()
{
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab37Text::StaggerLog>(GFrameCounter));
	GetAnim()->Montage_Stop(0.1f, SlashMontage);
}

void ABH_Knight::OnAnimEvent(FName EventName)
{
	if (EventName == TEXT("HitOn"))
	{
		bHitboxOn = true;
	}
	else if (EventName == TEXT("HitOff"))
	{
		bHitboxOn = false;
	}
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab37Text::EventLog>(GFrameCounter, *EventName.ToString(), bHitboxOn ? BugHunt::Tr<Lab37Text::On>() : BugHunt::Tr<Lab37Text::Off>()));
}

void ABH_Lab37::StartLab()
{
	ABH_Knight* Knight = SpawnLabActor<ABH_Knight>(FVector(300.f, 0.f, 100.f));
	const float Length = Knight->SlashMontage->GetPlayLength();
	const float HitAt = Length * 0.45f;
	BugHunt::Note(BugHunt::Fmt<Lab37Text::Plan>(
		Length, Length * 0.3f, Length * 0.6f, HitAt));

	Knight->Slash();
	After(HitAt - 0.05f, [Knight]
	{
		BugHunt::ExpectTrue(BugHunt::Tr<Lab37Text::Passing>(), Knight->bHitboxOn);
	});
	After(HitAt, [Knight] { Knight->Stagger(); });

	After(HitAt + 1.0f, [this, Knight]
	{
		BugHunt::Note(BugHunt::Fmt<Lab37Text::StillPlay>(
			Knight->GetAnim()->Montage_IsPlaying(Knight->SlashMontage) ? BugHunt::Tr<Lab37Text::Yes>() : BugHunt::Tr<Lab37Text::No>()));
		BugHunt::ExpectTrue(BugHunt::Tr<Lab37Text::AfterCut>(), !Knight->bHitboxOn);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 로그에서 HitOn 다음에 HitOff 가 찍혔나요? 경직은 히트박스 구간의 어디쯤에서 일어났나요?
// OnAnimEvent 에 중단점을 걸고, 경직 뒤에 한 번이라도 더 멈추는지 보세요.

// [JA] ログで HitOn の後に HitOff は記録されましたか？ 怯みはヒットボックス区間のどのあたりで起きましたか？
// [JA] OnAnimEvent にブレークポイントを置き、怯みの後にもう一度でも止まるか見てください。
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 노티파이는 몽타주가 그 시각을 "지나갈 때"만 옵니다. 몽타주가 그 전에 멈추면 HitOff 는 영영 오지 않습니다.
// 켜는 일과 끄는 일을 서로 다른 시각의 노티파이 두 개에 맡겼습니다. 둘 중 하나만 오는 경우는 어떤 때인가요?
// (경직, 사망, 회피, 다른 몽타주 재생, 렉으로 프레임이 크게 뛰어 몽타주가 끝나 버림...)

// [JA] 通知(Notify)は、モンタージュがその時刻を「通過するとき」にだけ来ます。モンタージュがその前に止まると、HitOff は永遠に来ません。
// [JA] オンにする処理とオフにする処理を、別々の時刻の 2 つの通知に任せています。どちらか一方だけが来るのはどんなときですか？
// [JA] （怯み、死亡、回避、別のモンタージュの再生、ラグでフレームが大きく飛んでモンタージュが終わってしまう...）
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 구간에 걸친 상태는 노티파이 "스테이트"(UAnimNotifyState)로 만듭니다. NotifyBegin 이 불렸다면 NotifyEnd 는
// 몽타주가 끝나든 끊기든 반드시 불립니다. 이 프로젝트의 BugHunt::AddNotify 에 Duration 을 주면 스테이트가 됩니다.
// 또는 몽타주가 끝나거나 끊길 때(OnMontageBlendingOut / OnMontageEnded) 히트박스를 끄는 안전장치를 둡니다.

// [JA] 区間にまたがる状態は、通知「ステート」（UAnimNotifyState）で作ります。NotifyBegin が呼ばれたなら、NotifyEnd は
// [JA] モンタージュが終わっても中断されても必ず呼ばれます。このプロジェクトの BugHunt::AddNotify に Duration を渡すとステートになります。
// [JA] または、モンタージュが終わったり中断されたりしたとき（OnMontageBlendingOut / OnMontageEnded）にヒットボックスをオフにする安全策を置きます。
#pragma endregion
