#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab06_Poison.generated.h"

// 독: 0.4초마다 5 피해. 독에 다시 걸리면 지속 시간이 처음부터 다시 시작한다 (중첩되지 않음).
// 정화하면 독이 즉시 사라진다.
UCLASS(NotPlaceable)
class ABH_PoisonVictim : public AActor
{
	GENERATED_BODY()

public:
	void ApplyPoison();
	void RefreshPoison();
	void Cleanse();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Health = 100;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	bool bPoisoned = false;

	// 정화한 뒤에 들어온 독 피해 횟수 (0이어야 한다)
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 TicksAfterCleanse = 0;

private:
	void PoisonTick();

	FTimerHandle PoisonTimer;
};

UCLASS(NotPlaceable)
class ABH_Lab06 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
