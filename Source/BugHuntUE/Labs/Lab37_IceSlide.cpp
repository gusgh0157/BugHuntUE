#include "Labs/Lab37_IceSlide.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

ABH_IceStone::ABH_IceStone()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ABH_IceStone::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	AddActorWorldOffset(FVector(Velocity * DeltaSeconds, 0.f, 0.f));
	Velocity *= Friction;
}

void ABH_Lab37::SlideAt(float Fps, float PushSpeed, TFunction<void(float)> Done)
{
	BugHunt::SetFixedFrameRate(Fps);
	ABH_IceStone* Stone = SpawnLabActor<ABH_IceStone>();
	After(0.f, [Stone, PushSpeed] { Stone->Push(PushSpeed); });
	After(3.f, [Stone, Fps, Done]
	{
		const float Distance = Stone->GetActorLocation().X;
		BugHunt::Note(FString::Printf(TEXT("%.0f fps: 미끄러진 거리 %.0f cm"), Fps, Distance));
		Done(Distance);
	});
}

void ABH_Lab37::StartLab()
{
	const float PushSpeed = static_cast<float>(BugHunt::CaseInt(1200, 600, 2400));
	// 60fps 에서 기획자가 본 거리: 매 프레임 (속도 / 60) 만큼 가고 속도가 0.9 배 → 합계 = 속도 / 60 / (1 - 0.9)
	const float Expected = PushSpeed / 6.f;
	BugHunt::Note(FString::Printf(TEXT("돌을 초속 %.0f cm 로 민다. 기획자가 60fps 에서 본 거리는 %.0f cm."), PushSpeed, Expected));

	SlideAt(30.f, PushSpeed, [this, PushSpeed, Expected](float At30)
	{
		SlideAt(120.f, PushSpeed, [this, Expected, At30](float At120)
		{
			const float Tolerance = Expected * 0.15f;
			BugHunt::ExpectTrue(FString::Printf(TEXT("30fps 미끄러진 거리 %.0f cm ≈ %.0f cm (±15%%)"), At30, Expected), FMath::Abs(At30 - Expected) <= Tolerance);
			BugHunt::ExpectTrue(FString::Printf(TEXT("120fps 미끄러진 거리 %.0f cm ≈ %.0f cm (±15%%)"), At120, Expected), FMath::Abs(At120 - Expected) <= Tolerance);
			FinishLab();
		});
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 이번엔 이동에 DeltaSeconds 를 곱했는데도 거리가 프레임레이트를 따라갑니다. 30fps 는 멀리, 120fps 는 짧게.
// Tick 에 추적점을 걸어 {Velocity} 를 찍어 보세요. 1초 뒤의 속도는 30fps 와 120fps 에서 각각 얼마인가요?
#pragma endregion

#pragma region 힌트 2
// Velocity *= 0.9 는 "Tick 한 번에 10% 감속" 입니다. 1초에 Tick 이 30번이면 0.9^30 ≈ 0.04, 120번이면 0.9^120 ≈ 0.000003.
// 감속(마찰, 공기 저항, 카메라 보간의 Lerp(A, B, 0.1) 도 마찬가지)도 시간에 맞춰야 합니다.
// 가속도를 적분하는 순서(속도 먼저냐 위치 먼저냐)도 프레임레이트에 따라 결과가 조금씩 달라집니다. 이 랩의 허용 오차(15%)는 그 차이 정도입니다.
#pragma endregion

#pragma region 힌트 3
// "60fps 에서 Tick 한 번에 0.9 배" 를 시간으로 바꾸면 "1/60 초마다 0.9 배" 입니다:
//     Velocity *= FMath::Pow(Friction, DeltaSeconds * 60.f);
// 처음부터 시간 기준으로 정의해도 됩니다: Velocity *= FMath::Exp(-Damping * DeltaSeconds); (Damping ≈ 6.3)
// 보간이라면 FMath::FInterpTo(Current, Target, DeltaSeconds, Speed) 가 같은 일을 합니다.
#pragma endregion
