#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "Lab28_QuestBoard.generated.h"

// 퀘스트 하나의 진행도. FastArraySerializer 원소라서 바뀐 원소만 클라이언트로 보낸다.
// [JA] クエスト 1 つの進行度。FastArraySerializer の要素なので、変わった要素だけをクライアントへ送る。
USTRUCT()
struct FBH_QuestEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	UPROPERTY()
	FName Quest;

	UPROPERTY()
	int32 Progress = 0;
};

USTRUCT()
struct FBH_QuestList : public FFastArraySerializer
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FBH_QuestEntry> Items;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FFastArraySerializer::FastArrayDeltaSerialize<FBH_QuestEntry, FBH_QuestList>(Items, DeltaParms, *this);
	}
};

template<>
struct TStructOpsTypeTraits<FBH_QuestList> : public TStructOpsTypeTraitsBase2<FBH_QuestList>
{
	enum { WithNetDeltaSerializer = true };
};

// 마을 게시판: 모든 플레이어가 같은 퀘스트 진행도를 본다.
// [JA] 村の掲示板: すべてのプレイヤーが同じクエスト進行度を見る。
UCLASS(NotPlaceable)
class ABH_QuestBoard : public AActor
{
	GENERATED_BODY()

public:
	ABH_QuestBoard();
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void AddQuest(FName Quest);
	void AddProgress(FName Quest, int32 Amount);
	int32 GetProgress(FName Quest) const;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_CheckBoard(const TArray<FName>& QuestNames, const TArray<int32>& ExpectedProgress);

	UPROPERTY(Replicated)
	FBH_QuestList Quests;
};

UCLASS(NotPlaceable)
class ABH_Lab28 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
