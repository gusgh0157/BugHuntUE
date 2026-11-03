#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab09_Archer.generated.h"

// 연습용 허수아비. HP가 0이 되면 스스로 사라진다.
UCLASS(NotPlaceable)
class ABH_Dummy : public AActor
{
	GENERATED_BODY()

public:
	// 화살을 맞는다. 이 화살로 쓰러졌으면 true.
	bool ReceiveArrow(int32 Damage);

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Health = 30;
};

// 궁수: 0.1초마다 화살을 한 발 쏜다. 표적이 없으면 가장 가까운 허수아비를 고른다.
UCLASS(NotPlaceable)
class ABH_Archer : public AActor
{
	GENERATED_BODY()

public:
	void StartShooting();
	void StopShooting();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TArray<TObjectPtr<ABH_Dummy>> Candidates;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<ABH_Dummy> CurrentTarget;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Score = 0;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 ArrowsShot = 0;

private:
	void Volley();
	ABH_Dummy* FindNearestTarget() const;

	FTimerHandle VolleyTimer;
};

UCLASS(NotPlaceable)
class ABH_Lab09 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
