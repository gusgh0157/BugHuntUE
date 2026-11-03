#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Core/BugHuntAnim.h"
#include "Lab35_Footsteps.generated.h"

class UAnimMontage;

// 걷는 NPC: 걷기 애니메이션의 발이 땅에 닿는 순간마다 Footstep 노티파이가 온다.
// 발자국마다 소리를 내고, 근처 적 AI가 그 소리를 듣는다(청각 감지). 그래서 발자국 수가 게임 판정에 쓰인다.
// [JA] 歩く NPC: 歩きアニメーションで足が地面に着く瞬間ごとに Footstep 通知(Notify)が来る。
// [JA] 足音ごとに音を出し、近くの敵 AI がその音を聞く（聴覚による検知）。そのため足音の数がゲームの判定に使われる。
UCLASS(NotPlaceable)
class ABH_Walker : public ABH_AnimCharacter
{
	GENERATED_BODY()

public:
	ABH_Walker();

	// 걷기 몽타주를 한 번 재생한다. 몽타주 길이(초)를 돌려준다.
	// [JA] 歩きモンタージュを 1 回再生する。モンタージュの長さ（秒）を返す。
	float Walk();

	virtual void OnAnimEvent(FName EventName) override;

	int32 Footsteps = 0;

	UPROPERTY()
	TObjectPtr<UAnimMontage> WalkMontage;

protected:
	virtual void BeginPlay() override;
};

UCLASS(NotPlaceable)
class ABH_Lab35 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
