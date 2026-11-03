#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab41_Knockback.generated.h"

class UBoxComponent;

// 망치에 맞으면 뒤로 밀려나는 거한.
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
class ABH_Lab41 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
