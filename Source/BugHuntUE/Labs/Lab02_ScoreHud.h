#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab02_ScoreHud.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FBH_OnScoreChanged, const FString& /*Player*/, int32 /*NewScore*/);

UCLASS()
class UBH_Scoreboard : public UObject
{
	GENERATED_BODY()

public:
	void AddScore(const FString& Player, int32 Points);
	int32 ScoreOf(const FString& Player) const;

	FBH_OnScoreChanged OnScoreChanged;

private:
	UPROPERTY()
	TMap<FString, int32> Scores;
};

class ABH_ScoreHud;

// 점수를 관리하고, 점수판 HUD를 띄운다.
UCLASS(NotPlaceable)
class ABH_ScoreManager : public AActor
{
	GENERATED_BODY()

public:
	ABH_ScoreManager();

	UBH_Scoreboard* GetScoreboard() const { return Scoreboard; }

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<ABH_ScoreHud> Hud;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<UBH_Scoreboard> Scoreboard;
};

// 점수판을 화면에 그린다. 여기서는 마지막으로 표시한 글자를 기억하기만 한다.
UCLASS(NotPlaceable)
class ABH_ScoreHud : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	FString DisplayedText;

protected:
	virtual void BeginPlay() override;

	void HandleScoreChanged(const FString& Player, int32 NewScore);
};

UCLASS(NotPlaceable)
class ABH_Lab02 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
