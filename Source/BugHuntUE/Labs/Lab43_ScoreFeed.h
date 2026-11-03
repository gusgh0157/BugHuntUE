#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab43_ScoreFeed.generated.h"

// 화면 오른쪽 위의 점수판. UObject 라서 게임 스레드에서만 만질 수 있다.
UCLASS(NotPlaceable)
class ABH_ScoreFeed : public AActor
{
	GENERATED_BODY()

public:
	void AddScore(const FString& Player, int32 Points);

	UPROPERTY()
	TMap<FString, int32> Scores;

	int32 Updates = 0;
};

UCLASS(NotPlaceable)
class ABH_Lab43 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
