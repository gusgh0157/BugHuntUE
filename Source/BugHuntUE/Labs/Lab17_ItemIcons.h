#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab17_ItemIcons.generated.h"

class UImage;
class UTexture2D;

USTRUCT()
struct FBH_ItemData
{
	GENERATED_BODY()

	UPROPERTY()
	FString Name;

	// 아이콘은 인벤토리를 열 때만 필요하므로 소프트 레퍼런스로 둔다.
	UPROPERTY()
	TSoftObjectPtr<UTexture2D> Icon;
};

// 인벤토리 한 칸. 스크롤할 때 칸 위젯을 새로 만들지 않고 재사용한다 (리스트 뷰 방식).
UCLASS()
class UBH_ItemSlot : public UObject
{
	GENERATED_BODY()

public:
	void Init();
	void ShowItem(const FBH_ItemData& Item);

	UPROPERTY()
	TObjectPtr<UImage> IconImage;

	UPROPERTY()
	FString ShownName;
};

UCLASS(NotPlaceable)
class ABH_Lab17 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;

	UPROPERTY()
	TObjectPtr<UBH_ItemSlot> Slot;
};
