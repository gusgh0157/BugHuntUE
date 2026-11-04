// [주의] 이 랩은 소리를 냅니다. 실행 전에 시스템 볼륨을 낮추고, 헤드폰을 쓴다면 귀에서 떼고 시작하세요.
//        (실습용 소리는 볼륨 배율 0.5 로 재생합니다.)
//
// 검사 방법: Explode() 가 돌려준 UAudioComponent 를 0.1초 뒤에 보고 "아직 재생 중인가"를 판정합니다.
// 그래서 고칠 때도 Explode() 는 재생 중인 컴포넌트를 돌려줘야 합니다. 컴포넌트를 돌려주지 않는 함수
// (UGameplayStatics::PlaySoundAtLocation 등)로 바꾸면 소리는 제대로 나도 검사가 확인할 수 없어 FAIL 입니다.
// [JA] [注意] このラボは音が出ます。実行前にシステムの音量を下げ、ヘッドホンを使っている場合は耳から離してから始めてください。
// [JA]        （実習用の音は音量倍率 0.5 で再生します。）
// [JA]
// [JA] 検査方法: Explode() が返した UAudioComponent を 0.1 秒後に見て、「まだ再生中か」を判定します。
// [JA] そのため、直すときも Explode() は再生中のコンポーネントを返す必要があります。コンポーネントを返さない関数
// [JA] （UGameplayStatics::PlaySoundAtLocation など）に変えると、音は正しく鳴っても検査が確認できず FAIL になります。

#include "Labs/Lab38_Explosion.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab38Text
{
BH_TEXT(Exploded,      "  %s 폭발!", "  %s 爆発！");
BH_TEXT(Caution,       "[주의] 소리가 납니다. 볼륨을 낮추세요.", "[注意] 音が出ます。音量を下げてください。");
BH_TEXT(AudioInfo,     "  오디오: UseSound %d, 메인 장치 %p, 월드 장치 %p", "  オーディオ: UseSound %d、メインデバイス %p、ワールドデバイス %p");
BH_TEXT(SkipName,      "랩 38", "ラボ 38");
BH_TEXT(NoAudioDevice, "오디오 장치가 없습니다 (-nosound 로 실행했거나 출력 장치가 없음).", "オーディオデバイスがありません（-nosound で実行したか、出力デバイスがありません）。");
BH_TEXT(Duration,      "폭발음 길이 %.2f초. 0.1초 뒤에도 들리고 있어야 한다.", "爆発音の長さ %.2f 秒。0.1 秒後も聞こえていなければならない。");
BH_TEXT(Component,     "폭발음 컴포넌트: %s", "爆発音のコンポーネント: %s");
BH_TEXT(NoComponent,   "없음", "なし");
BH_TEXT(AfterState,    "  0.1초 뒤: 컴포넌트 %s, 재생 중 %s", "  0.1 秒後: コンポーネント %s、再生中 %s");
BH_TEXT(Alive,         "살아 있음", "生きている");
BH_TEXT(Gone,          "사라짐", "消えた");
BH_TEXT(Yes,           "예", "はい");
BH_TEXT(No,            "아니오", "いいえ");
BH_TEXT(StillPlaying,  "0.1초 뒤에도 폭발음이 재생 중", "0.1 秒後も爆発音が再生中");
}

ABH_ExplosiveBarrel::ABH_ExplosiveBarrel()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<USoundBase> Ping(TEXT("/Engine/EngineSounds/1kSineTonePing.1kSineTonePing"));
	ExplosionSound = Ping.Object;
}

UAudioComponent* ABH_ExplosiveBarrel::Explode()
{
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab38Text::Exploded>(*GetName()));
	UAudioComponent* Audio = UGameplayStatics::SpawnSoundAttached(ExplosionSound, RootComponent);
	if (Audio)
	{
		Audio->SetVolumeMultiplier(0.5f);   // (실습용: 소리를 줄여 둠. 버그와 상관없음)
		// [JA] （実習用: 音を小さくしておく。バグとは関係ない）
	}
	Destroy();
	return Audio;
}

void ABH_Lab38::StartLab()
{
	BugHunt::Note(BugHunt::Tr<Lab38Text::Caution>());
	ABH_ExplosiveBarrel* Barrel = SpawnLabActor<ABH_ExplosiveBarrel>(FVector(300.f, 0.f, 50.f));

	const bool bHasAudio = GEngine && GEngine->UseSound() && GetWorld()->GetAudioDeviceRaw() != nullptr;
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab38Text::AudioInfo>(GEngine && GEngine->UseSound(),
		GEngine ? GEngine->GetMainAudioDeviceRaw() : nullptr, GetWorld()->GetAudioDeviceRaw()));
	if (!bHasAudio || !Barrel->ExplosionSound)
	{
		BugHunt::Skip(BugHunt::Tr<Lab38Text::SkipName>(), BugHunt::Tr<Lab38Text::NoAudioDevice>());
		FinishLab();
		return;
	}

	// 검사 환경 고정: 엔진은 1초 이하의 짧은 소리가 들을 수 없는 위치(리스너 밖)면 컴포넌트를 아예 만들지 않는다.
	// 카메라 위치나 헤드리스 실행에 결과가 흔들리지 않도록 이 최적화를 끈다. (버그와는 상관없음)
	// [JA] 検査環境の固定: エンジンは 1 秒以下の短い音が聞こえない位置（リスナーの範囲外）にあると、コンポーネントをそもそも作らない。
	// [JA] カメラ位置やヘッドレス実行で結果がぶれないよう、この最適化をオフにする。（バグとは関係ない）
	if (IConsoleVariable* DistanceOpt = IConsoleManager::Get().FindConsoleVariable(TEXT("au.SoundDistanceOptimizationLength")))
	{
		DistanceOpt->Set(0.f);
	}

	const float Duration = Barrel->ExplosionSound->GetDuration();
	BugHunt::Note(BugHunt::Fmt<Lab38Text::Duration>(Duration));

	UAudioComponent* RawAudio = Barrel->Explode();
	BugHunt::Note(BugHunt::Fmt<Lab38Text::Component>(RawAudio ? *RawAudio->GetPathName() : BugHunt::Tr<Lab38Text::NoComponent>()));
	TWeakObjectPtr<UAudioComponent> Audio = RawAudio;

	After(0.1f, [this, Audio]
	{
		const bool bStillPlaying = Audio.IsValid() && Audio->IsPlaying();
		UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab38Text::AfterState>(
			Audio.IsValid() ? BugHunt::Tr<Lab38Text::Alive>() : BugHunt::Tr<Lab38Text::Gone>(), bStillPlaying ? BugHunt::Tr<Lab38Text::Yes>() : BugHunt::Tr<Lab38Text::No>()));
		BugHunt::ExpectTrue(BugHunt::Tr<Lab38Text::StillPlaying>(), bStillPlaying);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 오디오 컴포넌트는 만들어졌는데 0.1초 뒤에 사라졌습니다. 누가 지웠을까요?
// Explode 의 Destroy() 줄에 중단점을 걸고, 넘기기 전에 조사식에 Audio->GetOwner() 와 Audio->GetAttachParent() 를 넣어 보세요.
// 콘솔: au.Debug.Sounds 1 (화면에 지금 재생 중인 소리 목록), stat soundwaves, au.3dVisualize.Enabled 1 (소리 위치 표시)

// [JA] オーディオコンポーネントは作られたのに、0.1 秒後に消えていました。誰が消したのでしょう？
// [JA] Explode の Destroy() の行にブレークポイントを設定し、ステップで進む前にウォッチに Audio->GetOwner() と Audio->GetAttachParent() を入れてみてください。
// [JA] コンソール: au.Debug.Sounds 1（画面に今再生中の音の一覧）、stat soundwaves、au.3dVisualize.Enabled 1（音の位置を表示）
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// SpawnSoundAttached 는 오디오 컴포넌트를 "붙인 대상의 액터"에 만듭니다. 액터가 파괴되면 그 액터의 컴포넌트도 함께 파괴되고,
// 컴포넌트가 파괴되면 재생 중이던 소리도 멈춥니다. Destroy() 다음 프레임부터는 소리가 없습니다.

// [JA] SpawnSoundAttached はオーディオコンポーネントを「アタッチ先のアクター」に作ります。アクターが破棄されると、そのアクターのコンポーネントも一緒に破棄され、
// [JA] コンポーネントが破棄されると再生中だった音も止まります。Destroy() の次のフレームからは音がありません。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 액터보다 오래 살아야 하는 소리는 액터에 붙이지 않습니다. 그 자리에서 울리고 끝나는 소리는 어느 함수로 재생할까요?
// (UGameplayStatics::SpawnSoundAtLocation. 또는 액터를 바로 지우지 말고 숨긴 뒤 SetLifeSpan)
// 함정 하나 더: SpawnSoundAtLocation(this, ...) 처럼 첫 인자(WorldContextObject)에 곧 사라질 액터를 넘기면,
// 엔진은 그 액터를 컴포넌트의 주인으로 삼습니다. 고친 뒤에도 로그의 "폭발음 컴포넌트" 경로에 누가 들어 있는지 확인하세요.
// 검사는 Explode() 가 돌려준 컴포넌트를 봅니다. PlaySoundAtLocation 은 컴포넌트를 돌려주지 않으므로, 소리는 나도 검사는 FAIL 입니다.

// [JA] アクターより長く生きるべき音は、アクターにアタッチしません。その場で鳴って終わる音は、どの関数で再生するでしょう？
// [JA] （UGameplayStatics::SpawnSoundAtLocation。またはアクターをすぐに消さず、隠してから SetLifeSpan）
// [JA] 落とし穴をもう 1 つ: SpawnSoundAtLocation(this, ...) のように第 1 引数（WorldContextObject）にもうすぐ消えるアクターを渡すと、
// [JA] エンジンはそのアクターをコンポーネントの持ち主にします。直した後も、ログの「爆発音のコンポーネント」のパスに誰が入っているか確認してください。
// [JA] 検査は Explode() が返したコンポーネントを見ます。PlaySoundAtLocation はコンポーネントを返さないので、音は鳴っても検査は FAIL です。
#pragma endregion
