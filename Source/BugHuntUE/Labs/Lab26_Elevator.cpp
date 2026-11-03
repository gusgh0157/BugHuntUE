#include "Labs/Lab26_Elevator.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntHang.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

ABH_Elevator::ABH_Elevator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ABH_Elevator::Call(float InTargetHeight, float InSpeed)
{
	TargetHeight = InTargetHeight;
	Speed = InSpeed;
	bArrived = false;
	SetActorTickEnabled(true);
}

void ABH_Elevator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	++TicksWhileMoving;
	Height = FMath::Min(TargetHeight, Height + Speed * DeltaSeconds);
	if (Height >= TargetHeight)
	{
		bArrived = true;
		SetActorTickEnabled(false);
		UE_LOG(LogBugHunt, Log, TEXT("  %.2f초: 엘리베이터 도착 (높이 %.0f, Tick %d번)"), GetWorld()->GetTimeSeconds(), Height, TicksWhileMoving);
		OnArrived.Broadcast();
	}
}

void ABH_Passenger::RideElevator(ABH_Elevator* Elevator, float TargetHeight, float Speed)
{
	Elevator->Call(TargetHeight, Speed);

	// 엘리베이터가 올 때까지 기다렸다가 탄다.
	BugHunt::FHangGuard Guard(TEXT("엘리베이터 기다리기"));
	while (!Elevator->bArrived)
	{
		BH_HANG_CHECK(Guard);   // 실습용 안전장치 (Core/BugHuntHang.h). 버그는 이 줄이 아니다.
		FPlatformProcess::Sleep(0.01f);   // CPU 를 덜 쓰게 잠깐 쉰다
	}
	Board(Elevator);
}

void ABH_Passenger::Board(ABH_Elevator* Elevator)
{
	bBoarded = true;
	bElevatorWasThere = Elevator->bArrived;
	UE_LOG(LogBugHunt, Log, TEXT("  %.2f초: 승객 탑승 (엘리베이터 높이 %.0f)"), GetWorld()->GetTimeSeconds(), Elevator->Height);
}

void ABH_Lab26::StartLab()
{
	const float TargetHeight = BugHunt::CaseFloat(300.f, 200.f, 600.f);
	const float Speed = BugHunt::CaseFloat(600.f, 400.f, 1000.f);

	ABH_Elevator* Elevator = SpawnLabActor<ABH_Elevator>();
	ABH_Passenger* Passenger = SpawnLabActor<ABH_Passenger>(FVector(0.f, 0.f, TargetHeight));
	BugHunt::Note(FString::Printf(TEXT("승객이 높이 %.0f 에서 엘리베이터를 부른다. 속도 %.0f 이면 %.2f초 뒤 도착."),
		TargetHeight, Speed, TargetHeight / Speed));

	const double CallStart = FPlatformTime::Seconds();
	Passenger->RideElevator(Elevator, TargetHeight, Speed);
	const double BlockedMs = (FPlatformTime::Seconds() - CallStart) * 1000.0;

	After(TargetHeight / Speed + 1.0f, [this, Elevator, Passenger, BlockedMs]
	{
		BugHunt::ExpectTrue(TEXT("승객이 탔다"), Passenger->bBoarded);
		BugHunt::ExpectTrue(TEXT("탈 때 엘리베이터가 와 있었다"), Passenger->bElevatorWasThere);
		BugHunt::ExpectTrue(FString::Printf(TEXT("엘리베이터를 부르는 동안 게임이 멈춘 시간 < 100 ms (실제: %.0f ms)"), BlockedMs),
			BlockedMs < 100.0);
		BugHunt::ExpectTrue(TEXT("엘리베이터 도착"), Elevator->bArrived);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 진짜 게임이라면 여기서 에디터가 굳습니다. 그 상황을 직접 겪어 보세요:
//   1) 콘솔에 BugHunt.HangBudgetSeconds 30 을 넣고 BugHunt.Lab 26 → 게임 화면이 30초 동안 굳습니다.
//   2) 굳어 있는 동안 VS 에서 디버그 → 모두 중단 (Ctrl+Alt+Break).
//   3) 디버그 → 창 → 스레드 (Ctrl+Alt+H). "GameThread" 를 더블클릭하고 호출 스택을 보세요. 어디서 돌고 있나요?
//   4) 조사식에 Elevator->Height, Elevator->TicksWhileMoving 을 넣고 F5 → 다시 모두 중단. 값이 변하나요?
#pragma endregion

#pragma region 힌트 2
// 엘리베이터를 움직이는 것은 Tick 입니다. Tick 은 누가, 어느 스레드에서 부르나요?
// 게임 스레드 하나가 "Tick 돌리기 → 그리기 → 다음 프레임"을 차례로 합니다. 그 게임 스레드가 while 안에서 기다리면
// 다음 프레임이 오지 않고, Tick 도 오지 않고, bArrived 는 영원히 false 입니다. Sleep 을 넣어도 마찬가지입니다.
// 게임 코드에서 "다른 객체가 바꿔 줄 값을 while 로 기다리기"는 거의 항상 이 버그입니다.
#pragma endregion

#pragma region 힌트 3
// 기다리지 말고 "도착하면 불러 달라"고 등록하세요. 엘리베이터에는 이미 OnArrived 델리게이트가 있습니다.
//     Elevator->Call(TargetHeight, Speed);
//     Elevator->OnArrived.AddUObject(this, &ABH_Passenger::Board, Elevator);
// (Board 에 인자를 더 넘길 수 있습니다: AddUObject 의 뒤쪽 인자는 페이로드.)
// 매 프레임 확인하는 방법도 됩니다: 타이머(SetTimer 0.05초 반복)나 이 액터의 Tick 에서 bArrived 를 보고 탑승.
// 핵심은 게임 스레드를 돌려주는 것입니다.
#pragma endregion
