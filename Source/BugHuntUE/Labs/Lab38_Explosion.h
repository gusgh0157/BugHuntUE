#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab38_Explosion.generated.h"

class UAudioComponent;
class USoundBase;

// 폭발 통: 터지면 폭발음을 내고 사라진다. 폭발음은 끝까지 들려야 한다.
// [JA] 爆発する樽: 爆発すると爆発音を出して消える。爆発音は最後まで聞こえなければならない。
UCLASS(NotPlaceable)
class ABH_ExplosiveBarrel : public AActor
{
	GENERATED_BODY()

public:
	ABH_ExplosiveBarrel();

	// 폭발음을 재생한 오디오 컴포넌트를 돌려준다 (검사용).
	// [JA] 爆発音を再生したオーディオコンポーネントを返す（検査用）。
	UAudioComponent* Explode();

	UPROPERTY(EditAnywhere, Category = "BugHunt")
	TObjectPtr<USoundBase> ExplosionSound;
};

UCLASS(NotPlaceable)
class ABH_Lab38 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
