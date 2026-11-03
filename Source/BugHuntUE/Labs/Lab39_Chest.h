#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "GameFramework/PlayerController.h"
#include "Lab39_Chest.generated.h"

class ABH_PlayerController;

// 보물 상자: 처음 연 플레이어에게 100 골드를 준다.
// [JA] 宝箱: 最初に開けたプレイヤーに 100 ゴールドを与える。
UCLASS(NotPlaceable)
class ABH_Chest : public AActor
{
	GENERATED_BODY()

public:
	ABH_Chest();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void Open(ABH_PlayerController* Opener);

	UPROPERTY(Replicated, VisibleAnywhere, Category = "BugHunt")
	bool bOpened = false;

	static constexpr int32 Reward = 100;
};

// 이 프로젝트의 플레이어 컨트롤러. 골드는 서버가 관리하고 본인에게만 복제된다.
// [JA] このプロジェクトのプレイヤーコントローラー。ゴールドはサーバーが管理し、本人にだけレプリケーションされる。
UCLASS()
class ABH_PlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 플레이어가 상자 앞에서 상호작용 키(E)를 눌렀다. 그 플레이어의 머신에서 실행된다.
	// [JA] プレイヤーが宝箱の前でインタラクトキー（E）を押した。そのプレイヤーのマシンで実行される。
	void InteractWithChest(ABH_Chest* Chest);

	UPROPERTY(Replicated, VisibleAnywhere, Category = "BugHunt")
	int32 Gold = 0;

	// 랩용: 서버가 이 플레이어에게 "E 키를 눌러라"라고 시킨다. (녹화된 입력 재생)
	// [JA] ラボ用: サーバーがこのプレイヤーに「E キーを押せ」と指示する。（録画した入力の再生）
	UFUNCTION(Client, Reliable)
	void Client_PressInteract(ABH_Chest* Chest);
};

UCLASS(NotPlaceable)
class ABH_Lab39 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;

	void RunScenario();
};
