#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab10_Balance.generated.h"

// 슬라임. 기획팀이 ini 로 튜닝할 수 있도록 Config 속성을 쓴다.
// [JA] スライム。企画チームが ini で調整できるように Config プロパティを使う。
UCLASS(Config = Game, NotPlaceable)
class ABH_Slime : public AActor
{
	GENERATED_BODY()

public:
	ABH_Slime();

	UPROPERTY(Config, VisibleAnywhere, Category = "BugHunt")
	int32 MaxHealth;

	UPROPERTY(Config, VisibleAnywhere, Category = "BugHunt")
	float MoveSpeed;
};

UCLASS(NotPlaceable)
class ABH_Lab10 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
