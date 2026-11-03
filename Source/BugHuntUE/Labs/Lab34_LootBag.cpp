#include "Labs/Lab34_LootBag.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

UBH_LootItem* ABH_LootBag::AddLoot(const FString& Name, int32 Value)
{
	UBH_LootItem* Item = NewObject<UBH_LootItem>(this);
	Item->ItemName = Name;
	Item->Value = Value;
	Items.Add(Item);
	RecentlySeen.Add(Item);
	return Item;
}

void ABH_LootBag::Drop(UBH_LootItem* Item)
{
	Items.Remove(Item);
}

int32 ABH_LootBag::TotalValue() const
{
	int32 Total = 0;
	for (const UBH_LootItem* Item : Items)
	{
		Total += Item->Value;
	}
	return Total;
}

void ABH_Lab34::StartLab()
{
	ABH_LootBag* Bag = SpawnLabActor<ABH_LootBag>();
	const int32 Count = BugHunt::CaseInt(4, 3, 9);
	int32 Expected = 0;
	TArray<TWeakObjectPtr<UBH_LootItem>> Watch;   // 검사용 (버그 없음): 아이템이 아직 살아 있나만 본다
	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Value = BugHunt::CaseInt(10 * (i + 1), 1, 200);
		Watch.Add(Bag->AddLoot(FString::Printf(TEXT("전리품%d"), i + 1), Value));
		Expected += Value;
	}
	const int32 Dropped = Count - 1;
	Expected -= Watch[Dropped]->Value;
	Bag->Drop(Watch[Dropped].Get());
	BugHunt::Note(FString::Printf(TEXT("전리품 %d개를 줍고 마지막 하나를 버렸다. ... 시간이 흐른다 (GC 실행) ..."), Count));

	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	int32 Alive = 0;
	for (int32 i = 0; i < Count; ++i)
	{
		Alive += (i != Dropped && Watch[i].IsValid()) ? 1 : 0;
	}
	int32 SeenAlive = 0;
	for (const TWeakObjectPtr<UBH_LootItem>& Seen : Bag->RecentlySeen)
	{
		SeenAlive += Seen.IsValid() ? 1 : 0;
	}
	BugHunt::Expect(TEXT("GC 뒤에도 살아 있는 가방 속 아이템"), Count - 1, Alive);
	BugHunt::Expect(TEXT("최근 본 목록에서 아직 유효한 아이템 (버린 것은 사라져야 함)"), Count - 1, SeenAlive);
	if (Alive == Count - 1)
	{
		BugHunt::Expect(TEXT("가방 가치 합계"), Expected, Bag->TotalValue());
	}
	else
	{
		BugHunt::Skip(TEXT("가방 가치 합계"), TEXT("가방이 해제된 메모리를 가리키고 있어 읽지 않음 (읽으면 크래시하거나 쓰레기 값)"));
	}
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// GC 한 번에 가방 속 아이템이 전부 사라졌습니다. 콘솔에서 obj list class=BH_LootItem 을 GC 전후로 쳐 보세요.
// CollectGarbage 줄 뒤에 중단점을 걸고 조사식에 Bag->Items 를 넣어 펼쳐 보세요. 원소의 ItemName 은 읽히나요?
#pragma endregion

#pragma region 힌트 2
// GC 는 UPROPERTY 로 표시된 참조(와 AddReferencedObjects 로 알린 참조)만 따라갑니다. 랩 4 는 멤버 포인터 하나였고,
// 이번엔 컨테이너입니다. TArray<UBH_LootItem*> 에 UPROPERTY 가 없으면 배열 안의 포인터는 GC 에게 보이지 않습니다.
// 반대로 RecentlySeen 은 TWeakObjectPtr 이라 UPROPERTY 가 없어도 안전합니다: 객체가 사라지면 그 원소가 무효가 될 뿐입니다.
#pragma endregion

#pragma region 힌트 3
//     UPROPERTY()
//     TArray<TObjectPtr<UBH_LootItem>> Items;
// TMap/TSet 도 같습니다: UPROPERTY() TMap<FName, TObjectPtr<UBH_LootItem>>. 단, 구조체 안의 배열이면 그 구조체도 USTRUCT 이고
// 멤버마다 UPROPERTY 가 있어야 합니다. 붙잡을 생각이 없는 목록은 TArray<TWeakObjectPtr<...>> (사용할 때 IsValid 확인).
#pragma endregion
