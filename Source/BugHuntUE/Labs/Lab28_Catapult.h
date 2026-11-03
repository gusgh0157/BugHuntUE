#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab28_Catapult.generated.h"

class USphereComponent;

// 물리로 굴러다니는 바위.
// [JA] 物理で転がり回る岩。
UCLASS(NotPlaceable)
class ABH_Boulder : public AActor
{
	GENERATED_BODY()

public:
	ABH_Boulder();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USphereComponent> Ball;
};

// 투석기. 팔 끝에 바위를 얹고 앞으로 휘두르다가 놓는다.
// [JA] 投石機。アームの先に岩を載せて前へ振り、途中で放す。
UCLASS(NotPlaceable)
class ABH_Catapult : public AActor
{
	GENERATED_BODY()

public:
	ABH_Catapult();
	virtual void Tick(float DeltaSeconds) override;

	void Swing(ABH_Boulder* InBoulder, const FVector& InArmVelocity);
	void Release();

	UPROPERTY()
	TObjectPtr<ABH_Boulder> Carried;

	FVector ArmTip = FVector::ZeroVector;
	FVector ArmVelocity = FVector::ZeroVector;     // 초당 cm
	// [JA] 毎秒 cm
};

UCLASS(NotPlaceable)
class ABH_Lab28 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
