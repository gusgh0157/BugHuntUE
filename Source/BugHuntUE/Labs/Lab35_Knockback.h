#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab35_Knockback.generated.h"

class UBoxComponent;

// 망치에 맞으면 뒤로 밀려나는 거한.
// [JA] ハンマーで殴られると後ろへ押し飛ばされる巨漢。
UCLASS(NotPlaceable)
class ABH_Brute : public AActor
{
	GENERATED_BODY()

public:
	ABH_Brute();

	void Knockback(const FVector& Direction, float Distance);

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<UBoxComponent> Body;
};

// 성벽.
// [JA] 城壁。
UCLASS(NotPlaceable)
class ABH_CastleWall : public AActor
{
	GENERATED_BODY()

public:
	ABH_CastleWall();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<UBoxComponent> Box;
};

UCLASS(NotPlaceable)
class ABH_Lab35 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
