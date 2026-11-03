#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab16_BountyBoard.generated.h"

// 현상금 게시판: 몬스터 종류별 사냥 수를 센다. 종류는 보통 몇 개 안 되므로
// 힙 할당을 피하려고 TInlineSetAllocator<4> 를 쓴다 (4종류까지는 액터 안의 버퍼).
UCLASS(NotPlaceable)
class ABH_BountyBoard : public AActor
{
	GENERATED_BODY()

public:
	// 의뢰 대상(Target)은 따로 센다. 나머지는 종류별로 센다.
	void RecordHunt(FName Target, const TArray<FName>& Killed);

	int32 KillsOf(FName Monster) const;

	TMap<FName, int32, TInlineSetAllocator<4>> Kills;
};

UCLASS(NotPlaceable)
class ABH_Lab16 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
