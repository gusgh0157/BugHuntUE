#include "Labs/Lab15_PickupSword.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab15Text
{
BH_TEXT(PickedLog, "  검을 주움 → 검 위치 (%.0f, %.0f, %.0f), 손 위치 (%.0f, %.0f, %.0f)", "  剣を拾った → 剣の位置 (%.0f, %.0f, %.0f)、手の位置 (%.0f, %.0f, %.0f)");
BH_TEXT(Scene,     "견습 기사가 (%.0f, %.0f) 에서 %.0f도 방향을 보고 서 있고, 검은 발밑 근처 (%.0f, %.0f) 에 %.0f도 돌아간 채 누워 있다.", "見習い騎士が (%.0f, %.0f) で %.0f 度の方向を向いて立っていて、剣は足元近くの (%.0f, %.0f) に %.0f 度回った状態で横たわっている。");
BH_TEXT(Picked,    "주운 직후", "拾った直後");
BH_TEXT(Walked,    "3m 걸어간 뒤", "3m 歩いた後");
BH_TEXT(InHand,    "%s: 검이 손에 있다 (손과의 거리 %.0f cm)", "%s: 剣が手にある（手との距離 %.0f cm）");
BH_TEXT(Aligned,   "%s: 검날이 손과 같은 방향 (손 앞 방향과의 내적 %.2f)", "%s: 刃が手と同じ向き（手の前方向との内積 %.2f）");
BH_TEXT(Scale,     "검의 크기가 그대로 (월드 스케일 %.2f)", "剣の大きさがそのまま（ワールドスケール %.2f）");
}

ABH_Sword::ABH_Sword()
{
	Blade = CreateDefaultSubobject<USceneComponent>(TEXT("Blade"));
	RootComponent = Blade;
}

ABH_Squire::ABH_Squire()
{
	Body = CreateDefaultSubobject<USceneComponent>(TEXT("Body"));
	RootComponent = Body;
	Hand = CreateDefaultSubobject<USceneComponent>(TEXT("Hand"));
	Hand->SetupAttachment(Body);
	Hand->SetRelativeLocation(FVector(40.f, 30.f, 120.f));
	SetActorScale3D(FVector(1.25f));
}

void ABH_Squire::PickUp(ABH_Sword* Sword)
{
	// 검을 손에 붙인다. 붙이는 순간 검이 화면에서 튀지 않도록 월드 트랜스폼을 유지한다.
	// [JA] 剣を手に付ける。付けた瞬間に剣が画面上で飛ばないよう、ワールド トランスフォームを保つ。
	Sword->AttachToComponent(Hand, FAttachmentTransformRules::KeepWorldTransform);
	Held = Sword;
	const FVector S = Sword->GetActorLocation();
	const FVector H = Hand->GetComponentLocation();
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab15Text::PickedLog>(S.X, S.Y, S.Z, H.X, H.Y, H.Z));
}

namespace
{
void CheckSwordInHand(const ABH_Squire* Squire, const TCHAR* When)
{
	const FVector SwordAt = Squire->Held->GetActorLocation();
	const float Off = FVector::Dist(SwordAt, Squire->Hand->GetComponentLocation());
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab15Text::InHand>(When, Off), Off < 1.f);
	const float Dot = FVector::DotProduct(Squire->Held->GetActorForwardVector(), Squire->Hand->GetForwardVector());
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab15Text::Aligned>(When, Dot), Dot > 0.99f);
}
}

void ABH_Lab15::StartLab()
{
	const FVector SquireAt(static_cast<float>(BugHunt::CaseInt(800, -2000, 2000)), static_cast<float>(BugHunt::CaseInt(-600, -2000, 2000)), 0.f);
	const float SquireYaw = static_cast<float>(BugHunt::CaseInt(30, 0, 359));
	const FVector SwordAt = SquireAt + FVector(static_cast<float>(BugHunt::CaseInt(60, -80, 80)), static_cast<float>(BugHunt::CaseInt(-50, -80, 80)), 2.f);
	const float SwordYaw = static_cast<float>(BugHunt::CaseInt(200, 0, 359));
	BugHunt::Note(BugHunt::Fmt<Lab15Text::Scene>(SquireAt.X, SquireAt.Y, SquireYaw, SwordAt.X, SwordAt.Y, SwordYaw));

	ABH_Squire* Squire = SpawnLabActor<ABH_Squire>(SquireAt);
	Squire->SetActorRotation(FRotator(0.f, SquireYaw, 0.f));
	ABH_Sword* Sword = SpawnLabActor<ABH_Sword>(SwordAt);
	Sword->SetActorRotation(FRotator(0.f, SwordYaw, 0.f));

	Squire->PickUp(Sword);
	CheckSwordInHand(Squire, BugHunt::Tr<Lab15Text::Picked>());

	// 검을 들고 앞으로 3m 걷는다.
	// [JA] 剣を持って前へ 3m 歩く。
	Squire->AddActorWorldOffset(Squire->GetActorForwardVector() * 300.f);
	CheckSwordInHand(Squire, BugHunt::Tr<Lab15Text::Walked>());

	const float SwordScale = Sword->GetActorScale3D().X;
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab15Text::Scale>(SwordScale), FMath::IsNearlyEqual(SwordScale, 1.f, 0.01f));

	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 붙이기는 성공했습니다(걸어가면 검도 같이 움직입니다). 그런데 손이 아니라 손에서 떨어진 자리에 붙어 있습니다.
// PickUp 의 AttachToComponent 바로 뒤에서 멈추고 조사식에 Sword->GetRootComponent()->GetRelativeLocation() 을 넣어 보세요.
// 손 기준으로 검이 어디에 있다고 기록됐나요? 그 값은 어디서 왔을까요?

// [JA] 取り付け自体は成功しています（歩くと剣も一緒に動きます）。でも、手ではなく手から離れた場所に付いています。
// [JA] PickUp の AttachToComponent の直後で止めて、ウォッチに Sword->GetRootComponent()->GetRelativeLocation() を入れてみてください。
// [JA] 手を基準に、剣はどこにあると記録されましたか？ その値はどこから来たのでしょう？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// FAttachmentTransformRules 는 붙이는 순간 위치·회전·스케일을 각각 어떻게 할지 정합니다.
//   KeepWorld    : 붙이기 전 월드 트랜스폼을 유지 → 바닥에 있던 자리가 그대로 "손 기준 오프셋"이 된다
//   KeepRelative : 지금의 Relative 값(붙기 전이니 = 월드 좌표)을 손 기준으로 해석 → 더 멀리 튄다
//   SnapToTarget : 대상(손)의 트랜스폼으로 맞춤 (IncludingScale 이면 스케일까지 손을 따른다)
// 바닥에서 집어 드는 장비는 어느 규칙이어야 할까요? 견습 기사의 스케일이 1.25 라는 것도 잊지 마세요.

// [JA] FAttachmentTransformRules は、付けた瞬間に位置・回転・スケールをそれぞれどう扱うかを決めます。
// [JA]   KeepWorld    : 付ける前のワールド トランスフォームを保つ → 床にあった場所がそのまま「手基準のオフセット」になる
// [JA]   KeepRelative : 今の Relative 値（付ける前なので = ワールド座標）を手基準として解釈 → もっと遠くへ飛ぶ
// [JA]   SnapToTarget : 対象（手）のトランスフォームに合わせる（IncludingScale ならスケールまで手に従う）
// [JA] 床から拾い上げる装備はどのルールにすべきでしょう？ 見習い騎士のスケールが 1.25 であることも忘れないでください。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 손에 쥐는 물건은 위치·회전을 손에 맞추고 크기는 그대로 둡니다.
//     Sword->AttachToComponent(Hand, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
// 소켓에 붙일 때도 같습니다(AttachToComponent(Mesh, Rules, SocketName)). 붙인 뒤 손에서 조금 띄우고 싶으면
// SetRelativeLocation/Rotation 으로 손 기준 오프셋을 따로 줍니다.

// [JA] 手に握る物は、位置・回転を手に合わせて、大きさはそのままにします。
// [JA]     Sword->AttachToComponent(Hand, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
// [JA] ソケットに付けるときも同じです（AttachToComponent(Mesh, Rules, SocketName)）。付けた後に手から少しずらしたい場合は、
// [JA] SetRelativeLocation/Rotation で手基準のオフセットを別に与えます。
#pragma endregion
