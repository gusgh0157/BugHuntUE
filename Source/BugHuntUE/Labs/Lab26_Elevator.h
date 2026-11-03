#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab26_Elevator.generated.h"

DECLARE_MULTICAST_DELEGATE(FBH_OnElevatorArrived);

// 엘리베이터: 부르면 매 Tick 마다 Speed 만큼 올라와서 도착하면 bArrived 를 켜고 OnArrived 를 방송한다.
UCLASS(NotPlaceable)
class ABH_Elevator : public AActor
{
	GENERATED_BODY()

public:
	ABH_Elevator();

	void Call(float InTargetHeight, float InSpeed);

	virtual void Tick(float DeltaSeconds) override;

	FBH_OnElevatorArrived OnArrived;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	float Height = 0.f;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	bool bArrived = false;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 TicksWhileMoving = 0;

private:
	float TargetHeight = 0.f;
	float Speed = 0.f;
};

// 승객: 엘리베이터를 부르고, 도착하면 탄다.
UCLASS(NotPlaceable)
class ABH_Passenger : public AActor
{
	GENERATED_BODY()

public:
	void RideElevator(ABH_Elevator* Elevator, float TargetHeight, float Speed);

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	bool bBoarded = false;

	// 탈 때 엘리베이터가 도착해 있었나
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	bool bElevatorWasThere = false;

private:
	void Board(ABH_Elevator* Elevator);
};

UCLASS(NotPlaceable)
class ABH_Lab26 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
