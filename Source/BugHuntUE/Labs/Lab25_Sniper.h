#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab25_Sniper.generated.h"

class USphereComponent;
class UBoxComponent;

// 저격총 탄환. 물리로 날아가고, 무언가에 부딪히면 Hits 를 센다.
UCLASS(NotPlaceable)
class ABH_SniperRound : public AActor
{
	GENERATED_BODY()

public:
	ABH_SniperRound();

	void Fire(float Speed);

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USphereComponent> Ball;

	int32 Hits = 0;

private:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
};

// 얇은 철판 과녁.
UCLASS(NotPlaceable)
class ABH_SteelPlate : public AActor
{
	GENERATED_BODY()

public:
	ABH_SteelPlate();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<UBoxComponent> Box;
};

UCLASS(NotPlaceable)
class ABH_Lab25 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
