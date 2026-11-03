#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab23_PotionBelt.generated.h"

struct FBH_BeltSlot
{
	FName Item;
	int32 Count = 0;
};

// 허리띠 인벤토리. 칸은 보통 4개를 넘지 않아서, 힙 할당을 피하려고 TInlineAllocator<4> 를 쓴다.
// (4칸까지는 액터 안의 고정 버퍼에, 넘으면 힙으로 옮겨 간다.)
// [JA] ベルトのインベントリ。スロットは普通 4 つを超えないので、ヒープ割り当てを避けるために TInlineAllocator<4> を使う。
// [JA] （4 スロットまではアクター内の固定バッファーに、超えるとヒープへ移る。）
UCLASS(NotPlaceable)
class ABH_PotionBelt : public AActor
{
	GENERATED_BODY()

public:
	void AddItem(FName Item, int32 Count);
	int32 CountOf(FName Item) const;

	// 웨이브마다 물약을 하나 마시고, 떨어진 전리품을 하나 줍는다.
	// [JA] ウェーブごとにポーションを 1 つ飲み、落ちた戦利品を 1 つ拾う。
	void FightWaves(const TArray<FName>& Drops);

	TArray<FBH_BeltSlot, TInlineAllocator<4>> Slots;

private:
	int32 FindSlot(FName Item) const;
};

UCLASS(NotPlaceable)
class ABH_Lab23 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
