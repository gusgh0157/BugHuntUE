#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab11_Dash.generated.h"

// 돌진: 일정 시간 동안 앞으로 빠르게 달린다. 기획: 초당 600cm.
// [JA] 突進: 一定時間、前方へ素早く走る。企画: 毎秒 600cm。
UCLASS(NotPlaceable)
class ABH_Dasher : public AActor
{
	GENERATED_BODY()

public:
	ABH_Dasher();
	virtual void Tick(float DeltaSeconds) override;

	void StartDash(float Seconds);

	// 60fps 에서 맞춘 값: 한 프레임에 10cm = 초당 600cm
	// [JA] 60fps で合わせた値: 1 フレームに 10cm = 毎秒 600cm
	float DashSpeed = 10.f;

	bool bDashing = false;
	float DashTimeLeft = 0.f;
};

UCLASS(NotPlaceable)
class ABH_Lab11 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;

	void DashAt(float Fps, float DashSeconds, TFunction<void(float)> Done);
};
