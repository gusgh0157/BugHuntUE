#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab33_FogOfWar.generated.h"

// 전장의 안개: 밝혀진 칸 좌표를 TSet 에 모은다. Source 는 디버그용 (누가 이 칸을 밝혔나).
struct FBH_CellKey
{
	int32 X = 0;
	int32 Y = 0;
	FString Source;

	// 같은 좌표면 같은 칸
	bool operator==(const FBH_CellKey& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}

	friend uint32 GetTypeHash(const FBH_CellKey& Key)
	{
		return HashCombine(HashCombine(GetTypeHash(Key.X), GetTypeHash(Key.Y)), GetTypeHash(Key.Source));
	}
};

UCLASS(NotPlaceable)
class ABH_FogOfWar : public AActor
{
	GENERATED_BODY()

public:
	void Reveal(int32 X, int32 Y, const FString& Source);
	bool IsRevealed(int32 X, int32 Y) const;

	TSet<FBH_CellKey> Revealed;
};

UCLASS(NotPlaceable)
class ABH_Lab33 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
