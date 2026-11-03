#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab38_RoundStats.generated.h"

// 한 라운드의 플레이어 기록.
USTRUCT()
struct FBH_RoundStats
{
	GENERATED_BODY()

	UPROPERTY()
	FString Player;

	UPROPERTY()
	int32 Kills;

	UPROPERTY()
	int32 Deaths;
};

// 라운드 점수판. 라운드마다 기록을 새로 시작한다.
UCLASS(NotPlaceable)
class ABH_RoundBoard : public AActor
{
	GENERATED_BODY()

public:
	void StartRound(const TArray<FString>& Players);
	void RecordKill(int32 Killer, int32 Victim);

	UPROPERTY()
	int32 Round;

	UPROPERTY()
	TArray<FBH_RoundStats> Stats;
};

UCLASS(NotPlaceable)
class ABH_Lab38 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
