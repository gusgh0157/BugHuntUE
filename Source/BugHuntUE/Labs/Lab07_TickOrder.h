#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab07_TickOrder.generated.h"

// 출혈 상태의 영웅: 매 프레임 HP가 1씩 줄어 0에서 멈춘다.
// [JA] 出血状態の英雄: 毎フレーム HP が 1 ずつ減り、0 で止まる。
UCLASS(NotPlaceable)
class ABH_BleedingHero : public AActor
{
	GENERATED_BODY()

public:
	ABH_BleedingHero();
	virtual void Tick(float DeltaSeconds) override;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Health = 30;
};

// 영웅 머리 위 HP 바: 매 프레임 영웅의 HP를 읽어 표시한다.
// [JA] 英雄の頭上の HP バー: 毎フレーム英雄の HP を読んで表示する。
UCLASS(NotPlaceable)
class ABH_HealthBar : public AActor
{
	GENERATED_BODY()

public:
	ABH_HealthBar();
	virtual void Tick(float DeltaSeconds) override;

	void Attach(ABH_BleedingHero* InHero);

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<ABH_BleedingHero> Hero;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 DisplayedHealth = 0;
};

// 프레임 맨 끝(모든 Tick 이후)에 "이번 프레임 화면에 나간 HP 바 == 실제 HP" 인지 검사한다.
// [JA] フレームの最後（すべての Tick の後）に「このフレームで画面に出た HP バー == 実際の HP」かどうかを検査する。
UCLASS(NotPlaceable)
class ABH_Lab07 : public ABugHuntLab
{
	GENERATED_BODY()

public:
	ABH_Lab07();
	virtual void Tick(float DeltaSeconds) override;

protected:
	virtual void StartLab() override;

	UPROPERTY()
	TObjectPtr<ABH_BleedingHero> Hero;

	UPROPERTY()
	TObjectPtr<ABH_HealthBar> Bar;

	int32 FramesChecked = 0;
	int32 FramesWrong = 0;
	int32 BarWhenHeroDied = -1;
};
