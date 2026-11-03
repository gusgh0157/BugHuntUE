#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab01_Pickup.generated.h"

class USphereComponent;

UCLASS(NotPlaceable)
class ABH_Hero : public AActor
{
	GENERATED_BODY()

public:
	ABH_Hero();

	void Heal(int32 Amount);

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USphereComponent> Body;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Health = 10;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 MaxHealth = 100;
};

UCLASS(NotPlaceable)
class ABH_Pet : public AActor
{
	GENERATED_BODY()

public:
	ABH_Pet();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USphereComponent> Body;

	// 이 펫의 주인
	// [JA] このペットの主人
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<ABH_Hero> Master;
};

UCLASS(NotPlaceable)
class ABH_HealthPickup : public AActor
{
	GENERATED_BODY()

public:
	ABH_HealthPickup();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USphereComponent> Trigger;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 HealAmount = 30;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	void OnTouched(AActor* Toucher);
	void GiveHealth(ABH_Hero* Hero);
};

UCLASS(NotPlaceable)
class ABH_Lab01 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
