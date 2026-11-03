#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab30_PriceTable.generated.h"

// 상점: 가격표에 있는 아이템만 사 준다. 가격표에 없는 아이템(퀘스트 아이템 등)은 팔 수 없으니 건너뛴다.
UCLASS(NotPlaceable)
class ABH_Shop : public AActor
{
	GENERATED_BODY()

public:
	// 가방을 통째로 판다. 받은 골드를 돌려준다.
	int32 SellAll(const TArray<FName>& Bag);

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TMap<FName, int32> Prices;

	// 팔지 못하고 돌려준 아이템 수
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Refused = 0;
};

UCLASS(NotPlaceable)
class ABH_Lab30 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
