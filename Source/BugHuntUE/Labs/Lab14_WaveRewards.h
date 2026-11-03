#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab14_WaveRewards.generated.h"

// 웨이브 보상: 기획서의 웨이브 번호는 1번부터. Rewards 에는 웨이브 1, 2, 3... 의 보상 골드가 차례로 들어 있다.
// [JA] ウェーブ報酬: 企画書のウェーブ番号は 1 番から。Rewards にはウェーブ 1、2、3... の報酬ゴールドが順に入っている。
UCLASS(NotPlaceable)
class ABH_WaveRewards : public AActor
{
	GENERATED_BODY()

public:
	// 웨이브 1 ~ NumCleared 를 깼다. 깬 웨이브의 보상을 모두 받는다.
	// [JA] ウェーブ 1 〜 NumCleared をクリアした。クリアしたウェーブの報酬をすべて受け取る。
	void ClaimRewards(int32 NumCleared);

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TArray<int32> Rewards;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Gold = 0;
};

UCLASS(NotPlaceable)
class ABH_Lab14 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
