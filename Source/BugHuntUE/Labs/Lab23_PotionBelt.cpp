#include "Labs/Lab23_PotionBelt.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab23Text
{
BH_TEXT(NewSlot,     "  새 칸: %s (칸 %d개)", "  新しいスロット: %s（スロット %d 個）");
BH_TEXT(Drank,       "  물약 마심 -> 남은 물약 %d", "  ポーションを飲んだ -> 残りのポーション %d");
BH_TEXT(Plan,        "물약 %d개로 웨이브 %d개를 싸운다. 웨이브마다 물약 하나를 마시고 전리품을 줍는다.", "ポーション %d 個でウェーブ %d 個を戦う。ウェーブごとにポーションを 1 つ飲み、戦利品を拾う。");
BH_TEXT(PotionsLeft, "남은 물약", "残りのポーション");
BH_TEXT(SlotCount,   "벨트 칸 수", "ベルトのスロット数");
}

namespace
{
const FName Potion(TEXT("Potion"));
}

int32 ABH_PotionBelt::FindSlot(FName Item) const
{
	return Slots.IndexOfByPredicate([Item](const FBH_BeltSlot& Slot) { return Slot.Item == Item; });
}

void ABH_PotionBelt::AddItem(FName Item, int32 Count)
{
	const int32 Index = FindSlot(Item);
	if (Index != INDEX_NONE)
	{
		Slots[Index].Count += Count;
		return;
	}
	Slots.Add({ Item, Count });
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab23Text::NewSlot>(*Item.ToString(), Slots.Num()));
}

int32 ABH_PotionBelt::CountOf(FName Item) const
{
	const int32 Index = FindSlot(Item);
	return Index != INDEX_NONE ? Slots[Index].Count : 0;
}

void ABH_PotionBelt::FightWaves(const TArray<FName>& Drops)
{
	// 물약 칸은 웨이브마다 쓰니까 미리 잡아 둔다.
	// [JA] ポーションのスロットはウェーブごとに使うので、先に取っておく。
	FBH_BeltSlot& Potions = Slots[FindSlot(Potion)];

	for (const FName& Drop : Drops)
	{
		Potions.Count -= 1;
		UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab23Text::Drank>(Potions.Count));
		AddItem(Drop, 1);
	}
}

void ABH_Lab23::StartLab()
{
	ABH_PotionBelt* Belt = SpawnLabActor<ABH_PotionBelt>();
	const int32 StartPotions = BugHunt::CaseInt(5, 6, 12);
	Belt->AddItem(Potion, StartPotions);
	Belt->AddItem(TEXT("Arrow"), 20);
	Belt->AddItem(TEXT("Bomb"), 2);

	// 공개 입력: Gem(새 칸), Arrow, Herb(새 칸), Gem
	// [JA] 公開入力: Gem（新しいスロット）、Arrow、Herb（新しいスロット）、Gem
	TArray<FName> Drops = { TEXT("Gem"), TEXT("Arrow"), TEXT("Herb"), TEXT("Gem") };
	if (BugHunt::IsHiddenCase())
	{
		const FName Pool[] = { TEXT("Gem"), TEXT("Herb"), TEXT("Ore"), TEXT("Arrow"), TEXT("Bomb") };
		const int32 NumDrops = BugHunt::CaseInt(0, 4, StartPotions - 1);
		Drops.Reset();
		for (int32 i = 0; i < NumDrops; ++i)
		{
			Drops.Add(Pool[BugHunt::CaseInt(0, 0, UE_ARRAY_COUNT(Pool) - 1)]);
		}
		// 세 번째 전리품까지 새 아이템이 두 종류 나오게 한다 (그 뒤에도 웨이브가 남아 있게)
		// [JA] 3 つ目の戦利品までに新しいアイテムが 2 種類出るようにする（その後にもウェーブが残るように）
		Drops[BugHunt::CaseInt(0, 0, 1)] = TEXT("Gem");
		Drops[2] = TEXT("Ore");
	}

	int32 NewKinds = 0;
	TSet<FName> Seen = { Potion, TEXT("Arrow"), TEXT("Bomb") };
	for (const FName& Drop : Drops)
	{
		bool bAlreadyIn = false;
		Seen.Add(Drop, &bAlreadyIn);
		NewKinds += bAlreadyIn ? 0 : 1;
	}

	BugHunt::Note(BugHunt::Fmt<Lab23Text::Plan>(StartPotions, Drops.Num()));
	Belt->FightWaves(Drops);

	BugHunt::Expect(BugHunt::Tr<Lab23Text::PotionsLeft>(), StartPotions - Drops.Num(), Belt->CountOf(Potion));
	BugHunt::Expect(BugHunt::Tr<Lab23Text::SlotCount>(), 3 + NewKinds, Belt->Slots.Num());
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// Output Log 의 "남은 물약" 줄을 보세요. 줄어들기는 하는데 결과가 다릅니다. 언제부터 어긋나나요?
// "새 칸" 로그와 같이 보세요. 몇 번째 칸이 생긴 뒤부터인가요?
// FightWaves 에 중단점을 걸고 조사식에 &Potions 와 &Slots[0] 을 넣으세요. 반복할 때마다 두 주소를 비교하세요.

// [JA] アウトプットログの「残りのポーション」の行を見てください。減ってはいるのに結果が違います。いつからずれていますか？
// [JA] 「新しいスロット」のログと一緒に見てください。何番目のスロットができた後からですか？
// [JA] FightWaves にブレークポイントを置き、ウォッチに &Potions と &Slots[0] を入れてください。ループのたびに 2 つのアドレスを比べてください。
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// TArray 는 원소를 한 덩어리 메모리에 담습니다. 칸이 모자라면 더 큰 메모리를 새로 잡아 원소를 "옮깁니다".
// 옮긴 뒤에는 예전에 꺼내 둔 참조/포인터(Potions)가 예전 자리를 가리킵니다. 거기에 쓰면 아무도 안 보는 메모리에 쓰는 것입니다.
// 이 벨트는 TInlineAllocator<4> 라서 예전 자리가 액터 안의 버퍼로 남아 있어 죽지 않고 값만 사라집니다.
// 보통 TArray 라면 예전 자리는 해제된 힙 메모리입니다. 더 나쁩니다 (가끔 죽거나, 다른 데이터를 망가뜨림).
// 데이터 중단점(주소 &Potions.Count, 4바이트)을 걸어 보세요. 누가 진짜 물약 수를 바꾸나요?

// [JA] TArray は要素を 1 つながりのメモリに入れます。容量が足りなくなると、より大きなメモリを新しく確保して要素を「移します」。
// [JA] 移した後は、以前に取り出しておいた参照／ポインター（Potions）が以前の場所を指します。そこに書くと、誰も見ていないメモリに書くことになります。
// [JA] このベルトは TInlineAllocator<4> なので、以前の場所がアクター内のバッファーとして残っていて、落ちずに値だけが消えます。
// [JA] 普通の TArray なら、以前の場所は解放されたヒープメモリです。もっと悪いです（ときどき落ちたり、他のデータを壊したりする）。
// [JA] データ ブレークポイント（アドレス &Potions.Count、4 バイト）を置いてみてください。本当のポーション数を変えているのは誰ですか？
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// Add/Emplace/Insert/Reserve/SetNum 처럼 배열 크기를 바꿀 수 있는 호출 뒤에는 원소 참조를 다시 꺼내야 합니다.
// 참조 대신 인덱스를 들고 있다가 쓸 때마다 Slots[Index] 로 접근하세요. (인덱스는 Add 해도 그대로입니다.)
//     const int32 PotionIndex = FindSlot(Potion);
//     Slots[PotionIndex].Count -= 1;
// std::vector 의 "push_back 후 이터레이터/참조 무효화" 와 같은 규칙입니다.

// [JA] Add/Emplace/Insert/Reserve/SetNum のように配列のサイズを変えうる呼び出しの後では、要素の参照を取り出し直す必要があります。
// [JA] 参照の代わりにインデックスを持っておき、使うたびに Slots[Index] でアクセスしてください。（インデックスは Add してもそのままです。）
// [JA]     const int32 PotionIndex = FindSlot(Potion);
// [JA]     Slots[PotionIndex].Count -= 1;
// [JA] std::vector の「push_back 後のイテレーター／参照の無効化」と同じルールです。
#pragma endregion
