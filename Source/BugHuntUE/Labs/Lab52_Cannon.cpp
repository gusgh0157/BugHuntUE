#include "Labs/Lab52_Cannon.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab52Text
{
BH_TEXT(Scene,    "대포가 (0,0) 에서 정면(+X)을 보고 있다. 표적은 %d도 방향, %d cm 앞. 대포가 표적 쪽으로 돈 뒤 쏜다.", "大砲が (0,0) で正面（+X）を向いている。標的は %d 度の方向、%d cm 先。大砲が標的のほうへ回ってから撃つ。");
BH_TEXT(FiredLog, "  발사: 포신 방향 (%.2f, %.2f), 포탄 속도 (%.0f, %.0f)", "  発射: 砲身の向き (%.2f, %.2f)、砲弾の速度 (%.0f, %.0f)");
BH_TEXT(Facing,   "대포가 표적 쪽을 보고 있다 (내적 %.3f)", "大砲が標的のほうを向いている（内積 %.3f）");
BH_TEXT(Along,    "포탄이 포신 방향으로 날아갔다 (내적 %.3f)", "砲弾が砲身の向きに飛んだ（内積 %.3f）");
BH_TEXT(Hit,      "포탄이 표적을 맞혔다 (가장 가까운 거리 %.0f cm, 기준 50)", "砲弾が標的に当たった（最も近い距離 %.0f cm、基準 50）");
}

ABH_CannonBall::ABH_CannonBall()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Ball"));
}

void ABH_CannonBall::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetActorLocation(GetActorLocation() + Velocity * DeltaSeconds);
	ClosestToTarget = FMath::Min(ClosestToTarget, FVector::Dist(GetActorLocation(), Target));
}

ABH_Cannon::ABH_Cannon()
{
	Base = CreateDefaultSubobject<USceneComponent>(TEXT("Base"));
	RootComponent = Base;
}

void ABH_Cannon::AimAt(const FVector& Target)
{
	FVector To = Target - GetActorLocation();
	To.Z = 0.f;
	SetActorRotation(To.Rotation());
}

ABH_CannonBall* ABH_Cannon::Fire()
{
	// 포구(포신 끝, 1.5m 앞)에서 포탄을 쏜다.
	// [JA] 砲口（砲身の先、1.5m 前）から砲弾を撃つ。
	const FVector Muzzle = GetActorLocation() + GetActorForwardVector() * 150.f;
	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	ABH_CannonBall* Ball = GetWorld()->SpawnActor<ABH_CannonBall>(ABH_CannonBall::StaticClass(), Muzzle, FRotator::ZeroRotator, Params);
	Ball->Velocity = FVector::ForwardVector * MuzzleSpeed;
	const FVector Dir = GetActorForwardVector();
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab52Text::FiredLog>(Dir.X, Dir.Y, Ball->Velocity.X, Ball->Velocity.Y));
	return Ball;
}

void ABH_Lab52::StartLab()
{
	BugHunt::SetFixedFrameRate(60.f);

	const int32 Angle = BugHunt::CaseInt(90, 30, 330);
	const int32 Range = BugHunt::CaseInt(2000, 1200, 2800);
	BugHunt::Note(BugHunt::Fmt<Lab52Text::Scene>(Angle, Range));

	const FVector Origin(0.f, -3000.f, 1500.f);
	const FVector Target = Origin + FRotator(0.f, static_cast<float>(Angle), 0.f).Vector() * static_cast<float>(Range);
	ABH_Cannon* Cannon = SpawnLabActor<ABH_Cannon>(Origin);
	Cannon->AimAt(Target);

	const FVector ToTarget = (Target - Origin).GetSafeNormal();
	const float FacingDot = FVector::DotProduct(Cannon->GetActorForwardVector(), ToTarget);
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab52Text::Facing>(FacingDot), FacingDot > 0.999f);

	ABH_CannonBall* Ball = Cannon->Fire();
	Ball->Target = Target;
	const float AlongDot = FVector::DotProduct(Ball->Velocity.GetSafeNormal(), Cannon->GetActorForwardVector());
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab52Text::Along>(AlongDot), AlongDot > 0.999f);

	After(2.f, [this, Ball]
	{
		BugHunt::ExpectTrue(BugHunt::Fmt<Lab52Text::Hit>(Ball->ClosestToTarget), Ball->ClosestToTarget < 50.f);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 대포는 표적 쪽으로 제대로 돌았는데 포탄은 다른 쪽으로 갑니다. 표적이 정면(+X)에 있을 때만 맞습니다.
// 로그의 "포신 방향"과 "포탄 속도"를 비교하세요. 포탄 속도는 표적 각도를 바꿔도 늘 같은 값인가요?
// Fire 에서 Muzzle 을 계산하는 줄과 Velocity 를 정하는 줄은 같은 "앞"을 쓰나요?

// [JA] 大砲は標的のほうへ正しく回ったのに、砲弾は別の方向へ行きます。標的が正面（+X）にあるときだけ当たります。
// [JA] ログの「砲身の向き」と「砲弾の速度」を比べてください。砲弾の速度は標的の角度を変えても常に同じ値ですか？
// [JA] Fire で Muzzle を計算する行と Velocity を決める行は、同じ「前」を使っていますか？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// FVector::ForwardVector 는 상수 (1, 0, 0), 곧 "월드의 +X" 입니다. 액터가 어디를 보든 바뀌지 않습니다.
// GetActorForwardVector() 는 액터 회전을 적용한 앞 방향입니다(= GetActorQuat().GetForwardVector()).
// 같은 이름 "Forward" 가 하나는 월드 축, 하나는 액터 축입니다. Right/Up 도 마찬가지입니다.

// [JA] FVector::ForwardVector は定数 (1, 0, 0)、つまり「ワールドの +X」です。アクターがどこを向いても変わりません。
// [JA] GetActorForwardVector() はアクターの回転を適用した前方向です（= GetActorQuat().GetForwardVector()）。
// [JA] 同じ名前「Forward」でも、片方はワールドの軸、片方はアクターの軸です。Right/Up も同じです。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 포탄은 포신이 보는 방향으로 쏩니다.
//     Ball->Velocity = GetActorForwardVector() * MuzzleSpeed;
// 확인할 때는 DrawDebugDirectionalArrow 로 포신 방향과 속도를 같이 그려 보세요(PIE 에서 화살표 두 개가 갈라지면 이 버그).
// 테스트 표적을 정면(+X)에만 두면 월드 축과 액터 축이 같아서 놓칩니다.

// [JA] 砲弾は砲身が向いている方向へ撃ちます。
// [JA]     Ball->Velocity = GetActorForwardVector() * MuzzleSpeed;
// [JA] 確認するときは DrawDebugDirectionalArrow で砲身の向きと速度を一緒に描いてみてください（PIE で矢印 2 本が分かれたらこのバグ）。
// [JA] テスト用の標的を正面（+X）にだけ置くと、ワールドの軸とアクターの軸が同じなので見逃します。
#pragma endregion
