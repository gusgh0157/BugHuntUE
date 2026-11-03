#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab45_Minimap.generated.h"

// 플레이어. 맵에 하나뿐이다.
UCLASS(NotPlaceable)
class ABH_MapHero : public AActor
{
	GENERATED_BODY()

public:
	ABH_MapHero();
};

// 미니맵 아이콘. 매 프레임 플레이어와의 거리에 따라 아이콘을 흐리게 한다.
UCLASS(NotPlaceable)
class ABH_MapMarker : public AActor
{
	GENERATED_BODY()

public:
	ABH_MapMarker();
	virtual void Tick(float DeltaSeconds) override;

	// 엔진의 UGameplayStatics::GetAllActorsOfClass 와 같은 일을 하는 함수 (월드의 모든 액터를 훑는다). 버그 없음.
	// 몇 번 불렸는지 세려고 감쌌다.
	static ABH_MapHero* FindHero(const UWorld* World);
	static int32 SearchCount;

	float Opacity = 1.f;
};

UCLASS(NotPlaceable)
class ABH_Lab45 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
