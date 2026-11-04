#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab33_Sniper.generated.h"

class USphereComponent;
class UBoxComponent;

// 저격총 탄환. 물리로 날아가고, 무언가에 부딪히면 Hits 를 센다.
// [JA] 狙撃銃の弾丸。物理で飛び、何かにぶつかると Hits を数える。
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
// [JA] 薄い鉄板の的。
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
class ABH_Lab33 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
