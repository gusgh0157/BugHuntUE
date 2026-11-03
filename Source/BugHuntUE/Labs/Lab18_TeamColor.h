#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab18_TeamColor.generated.h"

class UMaterialInstanceDynamic;

// 팀 색 칠하기: 기획서의 팀 색(디자인 툴에서 고른 16진수 색)을 유닛 머티리얼의 Color 파라미터에 넣는다.
UCLASS(NotPlaceable)
class ABH_Lab18 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> UnitMaterial;
};
