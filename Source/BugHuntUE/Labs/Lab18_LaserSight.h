#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab18_LaserSight.generated.h"

// 포구에서 앞으로 레이저 조준선을 쏘는 전차. 포구 위치는 전차 기준 좌표로 기억한다.
// [JA] 砲口から前へレーザー照準線を出す戦車。砲口の位置は戦車基準の座標で覚えている。
UCLASS(NotPlaceable)
class ABH_Tank : public AActor
{
	GENERATED_BODY()

public:
	ABH_Tank();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USceneComponent> Hull;

	// 포구 표시용 컴포넌트 (디테일 패널에서 위치를 보기 쉽게 둔 것. 조준 계산에는 쓰지 않는다)
	// [JA] 砲口表示用のコンポーネント（詳細パネルで位置を見やすくするために置いたもの。照準の計算には使わない）
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USceneComponent> MuzzleMarker;

	// 포구 (전차 기준: 앞으로 3m, 위로 1.2m)
	// [JA] 砲口（戦車基準: 前へ 3m、上へ 1.2m）
	FVector MuzzleOffset = FVector(300.f, 0.f, 120.f);

	FVector LaserStart = FVector::ZeroVector;
	FVector LaserDir = FVector::ForwardVector;

	void UpdateLaser();
};

UCLASS(NotPlaceable)
class ABH_Lab18 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
