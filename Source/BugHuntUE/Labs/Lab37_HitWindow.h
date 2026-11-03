#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Core/BugHuntAnim.h"
#include "Lab37_HitWindow.generated.h"

class UAnimMontage;

// 기사: 베기 몽타주의 칼이 지나가는 구간에만 무기 히트박스를 켠다.
// HitOn 노티파이에서 켜고, HitOff 노티파이에서 끈다. 공격 중에 맞으면 경직(Stagger)으로 베기가 끊긴다.
UCLASS(NotPlaceable)
class ABH_Knight : public ABH_AnimCharacter
{
	GENERATED_BODY()

public:
	void Slash();
	void Stagger();

	virtual void OnAnimEvent(FName EventName) override;

	bool bHitboxOn = false;

	UPROPERTY()
	TObjectPtr<UAnimMontage> SlashMontage;

protected:
	virtual void BeginPlay() override;
};

UCLASS(NotPlaceable)
class ABH_Lab37 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
