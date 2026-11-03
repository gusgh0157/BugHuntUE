#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab19_LootBag.generated.h"

UCLASS()
class UBH_LootItem : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	FString ItemName;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Value = 0;
};

// 전리품 가방. 아이템 객체는 가방이 만들고 가방만 들고 있다.
UCLASS(NotPlaceable)
class ABH_LootBag : public AActor
{
	GENERATED_BODY()

public:
	UBH_LootItem* AddLoot(const FString& Name, int32 Value);
	void Drop(UBH_LootItem* Item);
	int32 TotalValue() const;

	TArray<UBH_LootItem*> Items;

	// "최근 본 아이템" 목록 (UI 용). 아이템을 붙잡아 두지 않는다.
	TArray<TWeakObjectPtr<UBH_LootItem>> RecentlySeen;
};

UCLASS(NotPlaceable)
class ABH_Lab19 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
