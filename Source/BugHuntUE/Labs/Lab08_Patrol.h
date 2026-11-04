#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab08_Patrol.generated.h"

class UStaticMeshComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogBugHuntPatrol, Log, All);

// 순찰병: 웨이포인트를 차례로 걸어서 돈다. 웨이포인트에 AcceptRadius 안으로 들어오면 도착으로 친다.
// [JA] 巡回兵: ウェイポイントを順番に歩いて回る。ウェイポイントの AcceptRadius の内側に入ったら到着とみなす。
UCLASS(NotPlaceable)
class ABH_Patroller : public AActor
{
	GENERATED_BODY()

public:
	ABH_Patroller();
	virtual void Tick(float DeltaSeconds) override;

	// 웨이포인트는 바닥 위의 점이다.
	// [JA] ウェイポイントは床の上の点。
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TArray<FVector> Waypoints;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 TargetIndex = 0;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 WaypointsReached = 0;

	UPROPERTY(EditAnywhere, Category = "BugHunt")
	float Speed = 400.f;

	UPROPERTY(EditAnywhere, Category = "BugHunt")
	float AcceptRadius = 50.f;

	// 캡슐 절반 높이: 액터 위치(=몸 중심)는 바닥보다 이만큼 위에 있다.
	// [JA] カプセルの半分の高さ: アクターの位置（= 体の中心）は床よりこれだけ上にある。
	static constexpr float HalfHeight = 90.f;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<UStaticMeshComponent> Mesh;
};

UCLASS(NotPlaceable)
class ABH_Lab08 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
