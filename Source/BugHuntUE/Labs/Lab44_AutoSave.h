#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab44_AutoSave.generated.h"

UCLASS(NotPlaceable)
class ABH_SaveHero : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	FString HeroName = TEXT("Kael");

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Level = 7;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Gold = 120;
};

// 자동 저장. 디스크 쓰기는 느리므로 작업 스레드(스레드 풀)에서 한다.
// [JA] オートセーブ。ディスクへの書き込みは遅いので、ワーカースレッド（スレッドプール）で行う。
UCLASS(NotPlaceable)
class ABH_SaveSystem : public AActor
{
	GENERATED_BODY()

public:
	// 지금 순간의 영웅 상태를 저장한다. 게임은 멈추지 않아야 한다.
	// [JA] 今この瞬間の英雄の状態を保存する。ゲームは止まってはいけない。
	void AutoSave(ABH_SaveHero* Hero);

	// 저장이 끝나면 게임 스레드에서 불린다.
	// [JA] 保存が終わるとゲームスレッドで呼ばれる。
	void OnSaved(const FString& Text);

	// 영웅 상태를 저장 파일 한 줄로
	// [JA] 英雄の状態をセーブファイルの 1 行に
	static FString Describe(const ABH_SaveHero* Hero);

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	bool bSaved = false;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	FString SavedText;

	// 디스크에 쓰는 데 걸리는 시간 (흉내)
	// [JA] ディスクへの書き込みにかかる時間（模擬）
	float WriteSeconds = 0.3f;
};

UCLASS(NotPlaceable)
class ABH_Lab44 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
