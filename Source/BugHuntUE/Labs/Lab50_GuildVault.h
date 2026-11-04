#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab50_GuildVault.generated.h"

// 길드 금고. 여러 퀘스트 보상이 동시에 들어온다.
// [JA] ギルド金庫。複数のクエスト報酬が同時に入ってくる。
struct FBH_GuildVault
{
	int32 Gold = 0;

	void Deposit(int32 Amount);
};

UCLASS(NotPlaceable)
class ABH_Lab50 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
