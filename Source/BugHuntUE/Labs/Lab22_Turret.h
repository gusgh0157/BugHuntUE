#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab22_Turret.generated.h"

UCLASS(NotPlaceable)
class ABH_TurretTarget : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Health = 100;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 HitsTaken = 0;
};

// 자동 포탑: 일정한 간격으로 표적을 쏜다. 부서지면(Destroy) 더 쏘지 않아야 한다.
UCLASS(NotPlaceable)
class ABH_Turret : public AActor
{
	GENERATED_BODY()

public:
	void Activate(ABH_TurretTarget* InTarget, float InInterval);

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<ABH_TurretTarget> Target;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 ShotsFired = 0;

	static constexpr int32 ShotDamage = 5;

private:
	void Fire();

	FTimerHandle FireTimer;
};

UCLASS(NotPlaceable)
class ABH_Lab22 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
