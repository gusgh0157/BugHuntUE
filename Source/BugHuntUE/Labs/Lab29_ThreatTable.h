#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab29_ThreatTable.generated.h"

// 보스의 어그로(위협 수치) 표. 때린 만큼 어그로가 쌓이고, 보스는 어그로가 가장 높은 영웅을 노린다.
// AI 코드와 같이 쓰는 보통 C++ 구조체다. UObject 가 아니어서 UPROPERTY 를 달 수 없다.
// [JA] ボスのヘイト（脅威値）表。殴った分だけヘイトが溜まり、ボスはヘイトが最も高い英雄を狙う。
// [JA] AI コードと一緒に使う普通の C++ 構造体だ。UObject ではないので UPROPERTY を付けられない。
struct FBH_ThreatTable
{
	void AddThreat(AActor* Source, int32 Amount);
	int32 ThreatOf(AActor* Source) const;

	// 어그로가 가장 높은 대상. 표가 비었으면 nullptr.
	// [JA] ヘイトが最も高い対象。表が空なら nullptr。
	AActor* TopTarget() const;

	TMap<AActor*, int32> Threat;
};

UCLASS(NotPlaceable)
class ABH_Raider : public AActor
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	FString DisplayName;
};

UCLASS(NotPlaceable)
class ABH_RaidBoss : public AActor
{
	GENERATED_BODY()

public:
	void TakeHit(ABH_Raider* Attacker, int32 Damage);

	FBH_ThreatTable ThreatTable;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	int32 Health = 1000;
};

UCLASS(NotPlaceable)
class ABH_Lab29 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
