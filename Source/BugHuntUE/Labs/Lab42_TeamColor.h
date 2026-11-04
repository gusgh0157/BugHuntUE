#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab42_TeamColor.generated.h"

class UMaterialInstanceDynamic;

// 팀 색 칠하기: 기획서의 팀 색(디자인 툴에서 고른 16진수 색)을 유닛 머티리얼의 Color 파라미터에 넣는다.
// [JA] チームカラーの塗り分け: 仕様書のチームカラー（デザインツールで選んだ 16 進数の色）をユニットのマテリアルの Color パラメーターに入れる。
UCLASS(NotPlaceable)
class ABH_Lab42 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> UnitMaterial;
};
