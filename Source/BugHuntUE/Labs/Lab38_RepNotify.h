#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab38_RepNotify.generated.h"

// 네트워크 영웅. HP는 서버가 정하고 모든 클라이언트에 복제된다.
// 각 머신은 HP가 바뀌면 자기 화면의 HP 바(HudHealth)를 갱신한다.
UCLASS(NotPlaceable)
class ABH_NetHero : public AActor
{
	GENERATED_BODY()

public:
	ABH_NetHero();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 서버에서만 부른다.
	void ApplyDamage(int32 Damage);

	UPROPERTY(ReplicatedUsing = OnRep_Health, VisibleAnywhere, Category = "BugHunt")
	int32 Health = 100;

	// 이 머신 화면에 그려진 HP 바 값
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 HudHealth = 100;

	// 서버가 "지금 각자 화면을 검사해"라고 모두에게 보낸다.
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CheckHud(int32 ExpectedHealth);

protected:
	UFUNCTION()
	void OnRep_Health();

	void UpdateHud();
};

UCLASS(NotPlaceable)
class ABH_Lab38 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
