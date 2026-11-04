#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab55_MovingPlatform.generated.h"

class UStaticMeshComponent;

// 좌우로 오가는 발판. 벽·바닥 액터의 생성자를 복사해서 만들었다.
// [JA] 左右に行き来する足場。壁・床アクターのコンストラクターをコピーして作った。
UCLASS(NotPlaceable)
class ABH_MovingPlatform : public AActor
{
	GENERATED_BODY()

public:
	ABH_MovingPlatform();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<UStaticMeshComponent> Mesh;

	float Speed = 200.f;     // cm/s
	FVector Start = FVector::ZeroVector;
};

UCLASS(NotPlaceable)
class ABH_Lab55 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
