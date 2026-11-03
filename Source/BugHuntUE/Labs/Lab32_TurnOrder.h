#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab32_TurnOrder.generated.h"

// 턴제 전투의 행동 순서. 민첩이 높은 순서로 정렬해 두고, 쓰러진 유닛은 순서에서 뺀다.
UCLASS(NotPlaceable)
class ABH_TurnOrder : public AActor
{
	GENERATED_BODY()

public:
	void RemoveUnit(const FString& Unit);
	FString Describe() const;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TArray<FString> Order;
};

UCLASS(NotPlaceable)
class ABH_Lab32 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
