#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab04_QuestLog.generated.h"

UCLASS()
class UBH_QuestLog : public UObject
{
	GENERATED_BODY()

public:
	virtual void AddEntry(const FString& Quest);
	virtual int32 NumEntries() const { return Entries.Num(); }

private:
	UPROPERTY()
	TArray<FString> Entries;
};

// 퀘스트를 주는 NPC. 게임 내내 퀘스트 기록을 들고 있다.
UCLASS(NotPlaceable)
class ABH_QuestGiver : public AActor
{
	GENERATED_BODY()

public:
	void AcceptQuest(const FString& Quest);
	int32 NumAccepted() const;

protected:
	virtual void BeginPlay() override;

	UBH_QuestLog* QuestLog = nullptr;
};

UCLASS(NotPlaceable)
class ABH_Lab04 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;

	UPROPERTY()
	TObjectPtr<ABH_QuestGiver> Giver;
};
