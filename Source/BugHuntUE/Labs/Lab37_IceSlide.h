#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab37_IceSlide.generated.h"

// 얼음 위의 돌: 밀면 미끄러지다가 마찰로 멈춘다.
UCLASS(NotPlaceable)
class ABH_IceStone : public AActor
{
	GENERATED_BODY()

public:
	ABH_IceStone();
	virtual void Tick(float DeltaSeconds) override;

	void Push(float Speed) { Velocity = Speed; }

	float Velocity = 0.f;          // 초당 cm
	float Friction = 0.9f;         // 기획자가 60fps 에서 손으로 맞춘 값
};

UCLASS(NotPlaceable)
class ABH_Lab37 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;

	void SlideAt(float Fps, float PushSpeed, TFunction<void(float)> Done);
};
