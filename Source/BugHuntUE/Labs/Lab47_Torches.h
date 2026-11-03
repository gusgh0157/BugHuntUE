#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab47_Torches.generated.h"

// 성벽의 횃불. 0.5초마다 불꽃이 일렁인다.
UCLASS(NotPlaceable)
class ABH_Torch : public AActor
{
	GENERATED_BODY()

public:
	ABH_Torch();
	virtual void Tick(float DeltaSeconds) override;

	void Flicker();

	float FlickerTimer = 0.f;
	int32 Flickers = 0;

	static int32 TickCalls;     // 계측용 (모든 횃불의 Tick 호출 수)
};

UCLASS(NotPlaceable)
class ABH_Lab47 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
