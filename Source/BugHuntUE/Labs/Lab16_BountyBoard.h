#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab16_BountyBoard.generated.h"

// 현상금 게시판: 몬스터 종류별 사냥 수를 센다. 종류는 보통 몇 개 안 되므로
// 힙 할당을 피하려고 TInlineSetAllocator<4> 를 쓴다 (4종류까지는 액터 안의 버퍼).
// [JA] 賞金首の掲示板: モンスターの種類ごとの討伐数を数える。種類はふつう数個しかないので、
// [JA] ヒープ割り当てを避けるために TInlineSetAllocator<4> を使う（4 種類まではアクター内のバッファ）。
UCLASS(NotPlaceable)
class ABH_BountyBoard : public AActor
{
	GENERATED_BODY()

public:
	// 의뢰 대상(Target)은 따로 센다. 나머지는 종류별로 센다.
	// [JA] 依頼対象（Target）は別に数える。残りは種類ごとに数える。
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
