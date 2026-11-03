#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab14_Explosion.generated.h"

class UAudioComponent;
class USoundBase;

// 폭발 통: 터지면 폭발음을 내고 사라진다. 폭발음은 끝까지 들려야 한다.
UCLASS(NotPlaceable)
class ABH_ExplosiveBarrel : public AActor
{
	GENERATED_BODY()

public:
	ABH_ExplosiveBarrel();

	// 폭발음을 재생한 오디오 컴포넌트를 돌려준다 (검사용).
	UAudioComponent* Explode();

	UPROPERTY(EditAnywhere, Category = "BugHunt")
	TObjectPtr<USoundBase> ExplosionSound;
};

UCLASS(NotPlaceable)
class ABH_Lab14 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
