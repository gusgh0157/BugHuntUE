#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab05_CombatLog.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FBH_OnDamaged, AActor* /*Victim*/, int32 /*Damage*/);

UCLASS(NotPlaceable)
class ABH_Fighter : public AActor
{
	GENERATED_BODY()

public:
	void TakeHit(int32 Damage);

	FBH_OnDamaged OnDamaged;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Health = 100;
};

// 전투 기록 창. 열려 있는 동안 받은 피해를 한 줄씩 적는다.
// 창 객체는 닫아도 지우지 않고 다시 연다 (UI 풀링).
UCLASS()
class UBH_CombatLogPanel : public UObject
{
	GENERATED_BODY()

public:
	void Open(ABH_Fighter* InTarget);
	void Close();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TArray<FString> Lines;

	// 지금까지 화면에 띄운 알림 수 (통계용)
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 NotificationsShown = 0;

private:
	void HandleDamaged(AActor* Victim, int32 Damage);

	UPROPERTY()
	TObjectPtr<ABH_Fighter> Target;
};

UCLASS(NotPlaceable)
class ABH_Lab05 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;

	UPROPERTY()
	TObjectPtr<UBH_CombatLogPanel> Panel;
};
