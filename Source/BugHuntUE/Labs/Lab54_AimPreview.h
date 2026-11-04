#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab54_AimPreview.generated.h"

// 수류탄 조준선: 매 프레임 포물선 위의 점들을 계산해서 그린다.
// [JA] グレネードの照準線: 毎フレーム放物線上の点を計算して描く。
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
	// [JA] 描画用の結果
	int32 LastNumPoints = 0;
	FVector LastLanding = FVector::ZeroVector;
};

UCLASS(NotPlaceable)
class ABH_Lab54 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
