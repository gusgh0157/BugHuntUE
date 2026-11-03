#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab03_Cleanup.generated.h"

UCLASS(NotPlaceable)
class ABH_Enemy : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	FString DisplayName;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Health = 0;
};

// 웨이브가 끝나면 죽은 적을 목록에서 빼고 월드에서 지운다.
UCLASS(NotPlaceable)
class ABH_WaveManager : public AActor
{
	GENERATED_BODY()

public:
	void CleanupDeadEnemies();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TArray<TObjectPtr<ABH_Enemy>> Enemies;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Removed = 0;
};

UCLASS(NotPlaceable)
class ABH_Lab03 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
