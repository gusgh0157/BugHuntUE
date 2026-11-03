#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab36_Dash.generated.h"

// 돌진: 일정 시간 동안 앞으로 빠르게 달린다. 기획: 초당 600cm.
UCLASS(NotPlaceable)
class ABH_Dasher : public AActor
{
	GENERATED_BODY()

public:
	ABH_Dasher();
	virtual void Tick(float DeltaSeconds) override;

	void StartDash(float Seconds);

	// 60fps 에서 맞춘 값: 한 프레임에 10cm = 초당 600cm
	float DashSpeed = 10.f;

	bool bDashing = false;
	float DashTimeLeft = 0.f;
};

UCLASS(NotPlaceable)
class ABH_Lab36 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;

	void DashAt(float Fps, float DashSeconds, TFunction<void(float)> Done);
};
