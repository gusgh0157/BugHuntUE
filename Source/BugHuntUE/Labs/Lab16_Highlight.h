#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab16_Highlight.generated.h"

class UStaticMeshComponent;

// 상자: 마우스를 올리면 빨갛게 강조된다. 모든 상자는 같은 머티리얼 에셋(BasicShapeMaterial)을 쓴다.
UCLASS(NotPlaceable)
class ABH_Crate : public AActor
{
	GENERATED_BODY()

public:
	ABH_Crate();

	void SetHighlighted(bool bOn);

	// 지금 이 상자 화면에 칠해지는 색 (머티리얼의 Color 파라미터)
	FLinearColor GetDisplayedColor() const;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<UStaticMeshComponent> Mesh;

	static const FName ColorParam;
};

UCLASS(NotPlaceable)
class ABH_Lab16 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
