#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab31_AlarmBell.generated.h"

class USoundBase;

// 경보 종: 적이 성문에 닿으면 서버가 종을 친다. 모든 플레이어가 종소리를 들어야 한다.
// [JA] 警報の鐘: 敵が城門に着くとサーバーが鐘を鳴らす。すべてのプレイヤーに鐘の音が聞こえなければならない。
UCLASS(NotPlaceable)
class ABH_AlarmBell : public AActor
{
	GENERATED_BODY()

public:
	ABH_AlarmBell();

	// 서버에서만 부른다.
	// [JA] サーバーでのみ呼ぶ。
	void Ring();

	UPROPERTY(EditAnywhere, Category = "BugHunt")
	TObjectPtr<USoundBase> RingSound;

	// 이 머신에서 종소리를 재생한 횟수
	// [JA] このマシンで鐘の音を再生した回数
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 RingsHeard = 0;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CheckHeard(int32 ExpectedRings);

protected:
	void PlayRingSound();
};

UCLASS(NotPlaceable)
class ABH_Lab31 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
