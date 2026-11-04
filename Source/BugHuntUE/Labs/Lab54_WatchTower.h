#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab54_WatchTower.generated.h"

class USphereComponent;
class UStaticMeshComponent;

// 반경 8m 안에 들어온 침입자를 알리는 감시탑. 기획: "탑은 가늘고 높게".
// [JA] 半径 8m 以内に入った侵入者を知らせる監視塔。企画: 「塔は細く高く」。
UCLASS(NotPlaceable)
class ABH_WatchTower : public AActor
{
	GENERATED_BODY()

public:
	ABH_WatchTower();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USceneComponent> Base;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USphereComponent> Alarm;

	// 탑 모양을 만든다: 굵기 0.5배, 높이 HeightScale 배
	// [JA] 塔の形を作る: 太さ 0.5 倍、高さ HeightScale 倍
	void Build(float HeightScale);

	int32 Alarms = 0;

private:
	UFUNCTION()
	void OnSpotted(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};

// 감시탑에 다가오는 침입자.
// [JA] 監視塔に近づく侵入者。
UCLASS(NotPlaceable)
class ABH_Intruder : public AActor
{
	GENERATED_BODY()

public:
	ABH_Intruder();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USphereComponent> Body;
};

UCLASS(NotPlaceable)
class ABH_Lab54 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
