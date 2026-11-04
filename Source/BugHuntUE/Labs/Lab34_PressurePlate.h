#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Engine/StaticMeshActor.h"
#include "Lab34_PressurePlate.generated.h"

class UBoxComponent;

// 상자를 올려 놓으면 문이 열리는 압력판.
// [JA] 箱を載せると扉が開く感圧板。
UCLASS(NotPlaceable)
class ABH_PressurePlate : public AActor
{
	GENERATED_BODY()

public:
	ABH_PressurePlate();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<UBoxComponent> Trigger;

	int32 Presses = 0;
	bool bDoorOpen = false;

private:
	UFUNCTION()
	void OnPressed(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

// 바닥.
// [JA] 床。
UCLASS(NotPlaceable)
class ABH_Ground : public AActor
{
	GENERATED_BODY()

public:
	ABH_Ground();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<UBoxComponent> Box;
};

// 밀거나 떨어뜨릴 수 있는 나무 상자. 레벨 디자이너가 쓰는 StaticMeshActor 를 그대로 물려받았다.
// [JA] 押したり落としたりできる木箱。レベルデザイナーが使う StaticMeshActor をそのまま継承した。
UCLASS(NotPlaceable)
class ABH_WoodCrate : public AStaticMeshActor
{
	GENERATED_BODY()

public:
	ABH_WoodCrate();
};

UCLASS(NotPlaceable)
class ABH_Lab34 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
