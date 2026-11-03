#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab29_WaveRewards.generated.h"

// 웨이브 보상: 기획서의 웨이브 번호는 1번부터. Rewards 에는 웨이브 1, 2, 3... 의 보상 골드가 차례로 들어 있다.
UCLASS(NotPlaceable)
class ABH_WaveRewards : public AActor
{
	GENERATED_BODY()

public:
	// 웨이브 1 ~ NumCleared 를 깼다. 깬 웨이브의 보상을 모두 받는다.
	void ClaimRewards(int32 NumCleared);

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TArray<int32> Rewards;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Gold = 0;
};

UCLASS(NotPlaceable)
class ABH_Lab29 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
