#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntAnim.h"
#include "Core/BugHuntLab.h"
#include "Lab21_Lantern.generated.h"

// 광부가 손에 드는 랜턴.
// [JA] 鉱夫が手に持つランタン。
UCLASS(NotPlaceable)
class ABH_Lantern : public AActor
{
	GENERATED_BODY()

public:
	ABH_Lantern();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USceneComponent> Light;
};

// 오른손에 랜턴을 드는 광부. 소켓 이름은 다른 프로젝트의 캐릭터 코드에서 가져왔다.
// [JA] 右手にランタンを持つ鉱夫。ソケット名はほかのプロジェクトのキャラクターコードから持ってきた。
UCLASS(NotPlaceable)
class ABH_Miner : public ABH_AnimCharacter
{
	GENERATED_BODY()

public:
	// 랜턴을 붙일 소켓
	// [JA] ランタンを付けるソケット
	FName LanternSocket = TEXT("RightHandSocket");

	void HoldLantern(ABH_Lantern* Lantern);
};

UCLASS(NotPlaceable)
class ABH_Lab21 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
