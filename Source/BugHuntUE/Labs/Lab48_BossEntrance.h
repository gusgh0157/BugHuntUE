#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab48_BossEntrance.generated.h"

class AStaticMeshActor;
class UStaticMesh;

// 보스 방. 플레이어가 문을 열면 보스가 등장한다.
UCLASS(NotPlaceable)
class ABH_BossArena : public AActor
{
	GENERATED_BODY()

public:
	ABH_BossArena();
	virtual void BeginPlay() override;

	// 플레이어가 문을 연 순간 (게임 플레이 중) 불린다.
	AStaticMeshActor* SpawnBoss();

	// 보스 외형. 무거운 에셋이라 레벨을 열 때 같이 불러 두지 않는다.
	UPROPERTY(EditAnywhere, Category = "BugHunt")
	TSoftObjectPtr<UStaticMesh> BossMesh;
};

UCLASS(NotPlaceable)
class ABH_Lab48 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
