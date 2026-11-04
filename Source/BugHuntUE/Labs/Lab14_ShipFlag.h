#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab14_ShipFlag.generated.h"

// 돛대 꼭대기에 깃발을 다는 배. 깃발은 배(루트)에 붙은 자식 컴포넌트라서 배가 움직이면 따라온다.
// [JA] マストの先に旗を掲げる船。旗は船（ルート）に付いた子コンポーネントなので、船が動くとついてくる。
UCLASS(NotPlaceable)
class ABH_Ship : public AActor
{
	GENERATED_BODY()

public:
	ABH_Ship();

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USceneComponent> Hull;

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USceneComponent> Flag;

	// 돛대 꼭대기 (배 기준: 앞으로 2m, 위로 8m)
	// [JA] マストの先（船基準: 前へ 2m、上へ 8m）
	FVector MastTop = FVector(200.f, 0.f, 800.f);

	// 항구에 들어올 때마다 그 항구의 깃발로 바꿔 단다.
	// [JA] 港に入るたびに、その港の旗に掲げ替える。
	void RaiseFlag(FName PortFlag);

	FName CurrentFlag;
};

UCLASS(NotPlaceable)
class ABH_Lab14 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
