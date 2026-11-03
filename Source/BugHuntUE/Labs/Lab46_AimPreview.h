#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab46_AimPreview.generated.h"

// 수류탄 조준선: 매 프레임 포물선 위의 점들을 계산해서 그린다.
UCLASS(NotPlaceable)
class ABH_AimPreview : public AActor
{
	GENERATED_BODY()

public:
	ABH_AimPreview();
	virtual void Tick(float DeltaSeconds) override;

	int32 NumPoints = 64;
	FVector ThrowVelocity = FVector(800.f, 0.f, 600.f);

	// 그리기용 결과
	int32 LastNumPoints = 0;
	FVector LastLanding = FVector::ZeroVector;
};

UCLASS(NotPlaceable)
class ABH_Lab46 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
