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
	// [JA] ゲームを Fps に固定する（コンソールの t.MaxFPS、コマンドラインの -benchmark -fps=N と同じ効果。Core/BugHuntProbe.h）
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

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 프레임레이트만 바꿨는데 거리가 바뀝니다. 직접 해 보세요: 콘솔 t.MaxFPS 30 → BugHunt.Lab 11, t.MaxFPS 120 → 다시. (끝나면 t.MaxFPS 0)
// ABH_Dasher::Tick 에 조건부 중단점(bDashing)을 걸고 DeltaSeconds 를 보세요. 30fps 와 120fps 에서 Tick 은 몇 번 불리나요?
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// Tick 은 "1초에 몇 번"이 정해져 있지 않습니다. 한 번 불릴 때 얼마나 움직일지는 "지난 Tick 뒤로 흐른 시간(DeltaSeconds)" 에 비례해야 합니다.
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// DashTimeLeft 는 DeltaSeconds 로 줄이면서, 움직임에는 DeltaSeconds 를 쓰지 않았습니다.
#pragma endregion

#pragma region 힌트 3 / ヒント 3
//     float DashSpeed = 600.f;   // 초당 cm

// [JA] フレームレートを変えただけなのに距離が変わります。自分で試してみてください: コンソールで t.MaxFPS 30 → BugHunt.Lab 11、t.MaxFPS 120 → もう一度。（終わったら t.MaxFPS 0）
// [JA] ABH_Dasher::Tick に条件付きブレークポイント（bDashing）を設定し、DeltaSeconds を見てください。30fps と 120fps で Tick は何回呼ばれますか？
//     AddActorWorldOffset(FVector(DashSpeed * DeltaSeconds, 0.f, 0.f));
// 단위를 이름에 적어 두면(DashSpeedCmPerSec) 다음 사람이 같은 실수를 안 합니다.
#pragma endregion

// [JA] Tick は「1 秒に何回」と決まっていません。1 回呼ばれたときにどれだけ動くかは「前の Tick から経過した時間（DeltaSeconds）」に比例しなければなりません。
// [JA] DashTimeLeft は DeltaSeconds で減らしているのに、移動には DeltaSeconds を使っていませんでした。

// [JA]     float DashSpeed = 600.f;   // 毎秒 cm
// [JA]     AddActorWorldOffset(FVector(DashSpeed * DeltaSeconds, 0.f, 0.f));
// [JA] 単位を名前に書いておけば（DashSpeedCmPerSec）、次の人が同じミスをしません。
