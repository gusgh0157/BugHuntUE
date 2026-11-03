#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Core/BugHuntAnim.h"
#include "Lab36_ComboSwing.generated.h"

class UAnimMontage;

// 검사: 공격 버튼을 누르면 휘두르기 몽타주를 재생한다. 휘두르는 중에 다시 누르면 처음부터 다시 휘두른다(연타).
// 휘두르기를 끝까지 마치면 콤보 게이지가 1 오른다. 휘두르는 동안(bAttacking)에는 이동과 회피가 막힌다.
UCLASS(NotPlaceable)
class ABH_Swordsman : public ABH_AnimCharacter
{
	GENERATED_BODY()

public:
	void Attack();

	bool bAttacking = false;
	int32 CompletedSwings = 0;

	UPROPERTY()
	TObjectPtr<UAnimMontage> SwingMontage;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted);
};

UCLASS(NotPlaceable)
class ABH_Lab36 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
