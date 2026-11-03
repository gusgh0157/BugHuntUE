#include "Labs/Lab11_Dash.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

ABH_Dasher::ABH_Dasher()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ABH_Dasher::StartDash(float Seconds)
{
	bDashing = true;
	DashTimeLeft = Seconds;
}

void ABH_Dasher::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!bDashing)
	{
		return;
	}
	AddActorWorldOffset(FVector(DashSpeed, 0.f, 0.f));
	DashTimeLeft -= DeltaSeconds;
	if (DashTimeLeft <= 0.f)
	{
		bDashing = false;
	}
}

void ABH_Lab11::DashAt(float Fps, float DashSeconds, TFunction<void(float)> Done)
{
	// 게임을 Fps 로 고정한다 (콘솔 t.MaxFPS, 명령줄 -benchmark -fps=N 과 같은 효과. Core/BugHuntProbe.h)
	BugHunt::SetFixedFrameRate(Fps);
	ABH_Dasher* Dasher = SpawnLabActor<ABH_Dasher>();
	After(0.f, [Dasher, DashSeconds] { Dasher->StartDash(DashSeconds); });
	After(DashSeconds + 0.5f, [Dasher, Fps, Done]
	{
		const float Distance = Dasher->GetActorLocation().X;
		BugHunt::Note(FString::Printf(TEXT("%.0f fps: 돌진 거리 %.0f cm"), Fps, Distance));
		Done(Distance);
	});
}

void ABH_Lab11::StartLab()
{
	const float DashSeconds = BugHunt::CaseInt(500, 400, 900) / 1000.f;
	const float Expected = 600.f * DashSeconds;
	BugHunt::Note(FString::Printf(TEXT("%.1f초 돌진 (기획: 초당 600cm → %.0f cm). 30fps PC 와 120fps PC 에서 한 번씩."), DashSeconds, Expected));

	DashAt(30.f, DashSeconds, [this, DashSeconds, Expected](float At30)
	{
		DashAt(120.f, DashSeconds, [this, Expected, At30](float At120)
		{
			const float Tolerance = Expected * 0.15f;
			BugHunt::ExpectTrue(FString::Printf(TEXT("30fps 돌진 거리 %.0f cm ≈ %.0f cm (±15%%)"), At30, Expected), FMath::Abs(At30 - Expected) <= Tolerance);
			BugHunt::ExpectTrue(FString::Printf(TEXT("120fps 돌진 거리 %.0f cm ≈ %.0f cm (±15%%)"), At120, Expected), FMath::Abs(At120 - Expected) <= Tolerance);
			FinishLab();
		});
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 프레임레이트만 바꿨는데 거리가 바뀝니다. 직접 해 보세요: 콘솔 t.MaxFPS 30 → BugHunt.Lab 11, t.MaxFPS 120 → 다시. (끝나면 t.MaxFPS 0)
// ABH_Dasher::Tick 에 조건부 중단점(bDashing)을 걸고 DeltaSeconds 를 보세요. 30fps 와 120fps 에서 Tick 은 몇 번 불리나요?
#pragma endregion

#pragma region 힌트 2
// Tick 은 "1초에 몇 번"이 정해져 있지 않습니다. 한 번 불릴 때 얼마나 움직일지는 "지난 Tick 뒤로 흐른 시간(DeltaSeconds)" 에 비례해야 합니다.
// DashTimeLeft 는 DeltaSeconds 로 줄이면서, 움직임에는 DeltaSeconds 를 쓰지 않았습니다.
#pragma endregion

#pragma region 힌트 3
//     float DashSpeed = 600.f;   // 초당 cm
//     AddActorWorldOffset(FVector(DashSpeed * DeltaSeconds, 0.f, 0.f));
// 단위를 이름에 적어 두면(DashSpeedCmPerSec) 다음 사람이 같은 실수를 안 합니다.
#pragma endregion
