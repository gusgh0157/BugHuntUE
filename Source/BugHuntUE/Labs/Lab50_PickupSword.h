#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab50_PickupSword.generated.h"

// 바닥에 떨어져 있는 검.
// [JA] 床に落ちている剣。
UCLASS(NotPlaceable)
class ABH_Sword : public AActor
{
	GENERATED_BODY()

public:
	ABH_Sword();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USceneComponent> Blade;
};

// 검을 주워 오른손에 쥐는 견습 기사. 덩치가 커서 액터 스케일이 1.25 다.
// [JA] 剣を拾って右手に握る見習い騎士。体が大きいのでアクターのスケールが 1.25。
UCLASS(NotPlaceable)
class ABH_Squire : public AActor
{
	GENERATED_BODY()

public:
	ABH_Squire();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USceneComponent> Body;

	// 오른손 (몸 기준: 앞으로 40, 오른쪽 30, 위로 120)
	// [JA] 右手（体基準: 前へ 40、右へ 30、上へ 120）
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USceneComponent> Hand;

	void PickUp(ABH_Sword* Sword);

	UPROPERTY()
	TObjectPtr<ABH_Sword> Held;
};

UCLASS(NotPlaceable)
class ABH_Lab50 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
