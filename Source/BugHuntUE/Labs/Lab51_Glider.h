#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab51_Glider.generated.h"

// 조종간으로 움직이는 글라이더. 조종사 기준으로 기수를 들고(피치) 날개를 기울인다(롤).
// [JA] 操縦桿で動くグライダー。パイロット基準で機首を上げ（ピッチ）、翼を傾ける（ロール）。
UCLASS(NotPlaceable)
class ABH_Glider : public AActor
{
	GENERATED_BODY()

public:
	ABH_Glider();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USceneComponent> Airframe;

	// 한 프레임의 조종간 입력 (도)
	// [JA] 1 フレーム分の操縦桿入力（度）
	void Steer(float PitchDeg, float RollDeg);
};

UCLASS(NotPlaceable)
class ABH_Lab51 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
