#include "Labs/Lab30_PriceTable.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

int32 ABH_Shop::SellAll(const TArray<FName>& Bag)
{
	int32 Total = 0;
	for (const FName& Item : Bag)
	{
		const int32 Price = Prices[Item];
		Total += Price;
		UE_LOG(LogBugHunt, Log, TEXT("  %s 판매 +%d"), *Item.ToString(), Price);
	}
	return Total;
}

void ABH_Lab30::StartLab()
{
	ABH_Shop* Shop = SpawnLabActor<ABH_Shop>();
	const FName Sellable[] = { TEXT("Herb"), TEXT("Pelt"), TEXT("Ore"), TEXT("Gem") };
	for (const FName& Item : Sellable)
	{
		Shop->Prices.Add(Item, BugHunt::CaseInt(0, 1, 99));
	}
	if (!BugHunt::IsHiddenCase())
	{
		Shop->Prices = { { TEXT("Herb"), 5 }, { TEXT("Pelt"), 12 }, { TEXT("Ore"), 20 }, { TEXT("Gem"), 75 } };
	}

	// 공개 입력: 약초, 가죽, 왕의 편지(퀘스트 아이템), 광석, 보석
	TArray<FName> Bag = { TEXT("Herb"), TEXT("Pelt"), TEXT("KingsLetter"), TEXT("Ore"), TEXT("Gem") };
	if (BugHunt::IsHiddenCase())
	{
		Bag.Reset();
		const int32 Count = BugHunt::CaseInt(0, 4, 10);
		for (int32 i = 0; i < Count; ++i)
		{
			Bag.Add(Sellable[BugHunt::CaseInt(0, 0, UE_ARRAY_COUNT(Sellable) - 1)]);
		}
		Bag.Insert(TEXT("KingsLetter"), BugHunt::CaseInt(0, 0, Bag.Num()));
		Bag.Insert(TEXT("OldMap"), BugHunt::CaseInt(0, 0, Bag.Num()));
	}

	int32 Expected = 0;
	int32 ExpectedRefused = 0;
	for (const FName& Item : Bag)
	{
		const int32* Price = Shop->Prices.Find(Item);
		Expected += Price ? *Price : 0;
		ExpectedRefused += Price ? 0 : 1;
	}
	BugHunt::Note(FString::Printf(TEXT("가방의 아이템 %d개를 판다. 가격표에 없는 것은 %d개."), Bag.Num(), ExpectedRefused));

	const int32 Gold = Shop->SellAll(Bag);
	BugHunt::Expect(TEXT("받은 골드"), Expected, Gold);
	BugHunt::Expect(TEXT("팔지 못하고 돌려준 아이템"), ExpectedRefused, Shop->Refused);
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// "Assertion failed: Pair != nullptr" 같은 줄에서 멈춥니다. 엔진 프레임(TMapBase::FindChecked) 아래로 내려가
// 처음 나오는 BugHuntUE.dll! 프레임을 보세요. 지역 창의 Item 은 무엇인가요? Prices 를 펼쳐 그 키가 있나요?
#pragma endregion

#pragma region 힌트 2
// TMap 의 operator[] 는 std::map 과 다릅니다. std::map 은 없는 키를 만들어 넣지만, TMap 의 [] 는 FindChecked 와 같아서
// 없는 키면 check 로 프로그램을 멈춥니다. "있다고 확신하는 키"에만 씁니다.
#pragma endregion

#pragma region 힌트 3
// 없을 수도 있는 키는 Find 로 찾고 nullptr 를 확인합니다.
//     if (const int32* Price = Prices.Find(Item)) { Total += *Price; } else { ++Refused; }
// 기본값이 있으면 Prices.FindRef(Item) (없으면 0), 없으면 새로 넣으려면 FindOrAdd.
#pragma endregion
