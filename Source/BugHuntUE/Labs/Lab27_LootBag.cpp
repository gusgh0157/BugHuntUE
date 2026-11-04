#include "Labs/Lab27_LootBag.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab27Text
{
BH_TEXT(LootName,   "전리품%d", "戦利品%d");
BH_TEXT(Picked,     "전리품 %d개를 줍고 마지막 하나를 버렸다. ... 시간이 흐른다 (GC 실행) ...", "戦利品を %d 個拾い、最後の 1 つを捨てた。... 時間が流れる（GC 実行）...");
BH_TEXT(AliveInBag, "GC 뒤에도 살아 있는 가방 속 아이템", "GC の後も生きているバッグ内のアイテム");
BH_TEXT(SeenValid,  "최근 본 목록에서 아직 유효한 아이템 (버린 것은 사라져야 함)", "最近見たリストでまだ有効なアイテム（捨てたものは消えるべき）");
BH_TEXT(BagTotal,   "가방 가치 합계", "バッグの価値の合計");
BH_TEXT(SkipReason, "가방이 해제된 메모리를 가리키고 있어 읽지 않음 (읽으면 크래시하거나 쓰레기 값)", "バッグが解放済みのメモリを指しているため読まない（読むとクラッシュするかゴミ値）");
}

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

void ABH_Lab27::StartLab()
{
	ABH_LootBag* Bag = SpawnLabActor<ABH_LootBag>();
	const int32 Count = BugHunt::CaseInt(4, 3, 9);
	int32 Expected = 0;
	TArray<TWeakObjectPtr<UBH_LootItem>> Watch;   // 검사용 (버그 없음): 아이템이 아직 살아 있나만 본다
	// [JA] 検査用（バグなし）: アイテムがまだ生きているかだけを見る
	for (int32 i = 0; i < Count; ++i)
	{
		const int32 Value = BugHunt::CaseInt(10 * (i + 1), 1, 200);
		Watch.Add(Bag->AddLoot(BugHunt::Fmt<Lab27Text::LootName>(i + 1), Value));
		Expected += Value;
	}
	const int32 Dropped = Count - 1;
	Expected -= Watch[Dropped]->Value;
	Bag->Drop(Watch[Dropped].Get());
	BugHunt::Note(BugHunt::Fmt<Lab27Text::Picked>(Count));

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
	BugHunt::Expect(BugHunt::Tr<Lab27Text::AliveInBag>(), Count - 1, Alive);
	BugHunt::Expect(BugHunt::Tr<Lab27Text::SeenValid>(), Count - 1, SeenAlive);
	if (Alive == Count - 1)
	{
		BugHunt::Expect(BugHunt::Tr<Lab27Text::BagTotal>(), Expected, Bag->TotalValue());
	}
	else
	{
		BugHunt::Skip(BugHunt::Tr<Lab27Text::BagTotal>(), BugHunt::Tr<Lab27Text::SkipReason>());
	}
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// GC 한 번에 가방 속 아이템이 전부 사라졌습니다. 콘솔에서 obj list class=BH_LootItem 을 GC 전후로 쳐 보세요.
// CollectGarbage 줄 뒤에 중단점을 걸고 조사식에 Bag->Items 를 넣어 펼쳐 보세요. 원소의 ItemName 은 읽히나요?

// [JA] GC 1 回でバッグ内のアイテムがすべて消えました。コンソールで obj list class=BH_LootItem を GC の前後に打ってみてください。
// [JA] CollectGarbage の行の後にブレークポイントを置き、ウォッチに Bag->Items を入れて展開してみてください。要素の ItemName は読めますか？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// GC 는 UPROPERTY 로 표시된 참조(와 AddReferencedObjects 로 알린 참조)만 따라갑니다. 랩 4 는 멤버 포인터 하나였고,
// 이번엔 컨테이너입니다. TArray<UBH_LootItem*> 에 UPROPERTY 가 없으면 배열 안의 포인터는 GC 에게 보이지 않습니다.
// 반대로 RecentlySeen 은 TWeakObjectPtr 이라 UPROPERTY 가 없어도 안전합니다: 객체가 사라지면 그 원소가 무효가 될 뿐입니다.

// [JA] GC は UPROPERTY で示された参照（と AddReferencedObjects で知らせた参照）だけをたどります。ラボ 4 はメンバー ポインター 1 つでしたが、
// [JA] 今回はコンテナーです。TArray<UBH_LootItem*> に UPROPERTY がなければ、配列の中のポインターは GC から見えません。
// [JA] 逆に RecentlySeen は TWeakObjectPtr なので、UPROPERTY がなくても安全です: オブジェクトが消えると、その要素が無効になるだけです。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
//     UPROPERTY()
//     TArray<TObjectPtr<UBH_LootItem>> Items;
// TMap/TSet 도 같습니다: UPROPERTY() TMap<FName, TObjectPtr<UBH_LootItem>>. 단, 구조체 안의 배열이면 그 구조체도 USTRUCT 이고
// 멤버마다 UPROPERTY 가 있어야 합니다. 붙잡을 생각이 없는 목록은 TArray<TWeakObjectPtr<...>> (사용할 때 IsValid 확인).

// [JA]     UPROPERTY()
// [JA]     TArray<TObjectPtr<UBH_LootItem>> Items;
// [JA] TMap/TSet も同じです: UPROPERTY() TMap<FName, TObjectPtr<UBH_LootItem>>。ただし、構造体の中の配列なら、その構造体も USTRUCT で、
// [JA] メンバーごとに UPROPERTY が必要です。つかんでおくつもりのないリストは TArray<TWeakObjectPtr<...>>（使うときに IsValid を確認）。
#pragma endregion
