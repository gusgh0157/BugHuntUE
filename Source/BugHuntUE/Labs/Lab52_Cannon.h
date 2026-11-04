#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab52_Cannon.generated.h"

// 대포알. 중력 없이 곧게 날아간다(마법 대포).
// [JA] 砲弾。重力なしでまっすぐ飛ぶ（魔法の大砲）。
UCLASS(NotPlaceable)
class ABH_CannonBall : public AActor
{
	GENERATED_BODY()

public:
	ABH_CannonBall();
	virtual void Tick(float DeltaSeconds) override;

	FVector Velocity = FVector::ZeroVector;

	// 표적까지 가장 가까웠던 거리 (채점용)
	// [JA] 標的に最も近づいた距離（採点用）
	FVector Target = FVector::ZeroVector;
	float ClosestToTarget = TNumericLimits<float>::Max();
};

// 표적을 향해 돌아선 뒤 대포알을 쏘는 대포.
// [JA] 標的のほうへ向き直ってから砲弾を撃つ大砲。
UCLASS(NotPlaceable)
class ABH_Cannon : public AActor
{
	GENERATED_BODY()

public:
	ABH_Cannon();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USceneComponent> Base;

	float MuzzleSpeed = 2000.f;

	void AimAt(const FVector& Target);
	ABH_CannonBall* Fire();
};

UCLASS(NotPlaceable)
class ABH_Lab52 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
