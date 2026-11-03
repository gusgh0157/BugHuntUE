#include "Labs/Lab17_TurnOrder.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab17Text
{
BH_TEXT(Rogue,     "도적",     "盗賊");
BH_TEXT(Archer,    "궁수",     "弓兵");
BH_TEXT(Knight,    "기사",     "騎士");
BH_TEXT(Mage,      "마법사",   "魔法使い");
BH_TEXT(Priest,    "사제",     "僧侶");
BH_TEXT(Spearman,  "창병",     "槍兵");
BH_TEXT(Swordsman, "검사",     "剣士");
BH_TEXT(Druid,     "드루이드", "ドルイド");
BH_TEXT(Order,     "행동 순서 (민첩 순): %s", "行動順（素早さ順）: %s");
BH_TEXT(Fell,      "%s 쓰러짐", "%s が倒れた");
BH_TEXT(Remaining, "남은 유닛 수", "残りのユニット数");
BH_TEXT(Matches,   "행동 순서 == %s (실제: %s)", "行動順 == %s（実際: %s）");
}

void ABH_TurnOrder::RemoveUnit(const FString& Unit)
{
	const int32 Index = Order.IndexOfByKey(Unit);
	if (Index != INDEX_NONE)
	{
		Order.RemoveAtSwap(Index);   // 가운데를 빼도 뒤를 당기지 않아서 빠르다
		// [JA] 途中を抜いても後ろを詰めないので速い
	}
}

FString ABH_TurnOrder::Describe() const
{
	return FString::Join(Order, TEXT(" → "));
}

void ABH_Lab17::StartLab()
{
	ABH_TurnOrder* Turns = SpawnLabActor<ABH_TurnOrder>();
	TArray<FString> Units = { BugHunt::Tr<Lab17Text::Rogue>(), BugHunt::Tr<Lab17Text::Archer>(), BugHunt::Tr<Lab17Text::Knight>(), BugHunt::Tr<Lab17Text::Mage>(), BugHunt::Tr<Lab17Text::Priest>() };
	TArray<FString> Fallen = { BugHunt::Tr<Lab17Text::Archer>() };
	if (BugHunt::IsHiddenCase())
	{
		const TCHAR* Pool[] = { BugHunt::Tr<Lab17Text::Rogue>(), BugHunt::Tr<Lab17Text::Archer>(), BugHunt::Tr<Lab17Text::Knight>(), BugHunt::Tr<Lab17Text::Mage>(), BugHunt::Tr<Lab17Text::Priest>(), BugHunt::Tr<Lab17Text::Spearman>(), BugHunt::Tr<Lab17Text::Swordsman>(), BugHunt::Tr<Lab17Text::Druid>() };
		Units.Reset();
		const int32 Count = BugHunt::CaseInt(0, 5, UE_ARRAY_COUNT(Pool));
		for (int32 i = 0; i < Count; ++i)
		{
			Units.Add(Pool[i]);
		}
		// 맨 끝이 아닌 유닛 두 명이 쓰러진다
		// [JA] 最後尾ではないユニット 2 人が倒れる
		Fallen = { Units[BugHunt::CaseInt(0, 0, 1)], Units[BugHunt::CaseInt(0, 2, Count - 2)] };
	}

	Turns->Order = Units;
	BugHunt::Note(BugHunt::Fmt<Lab17Text::Order>(*Turns->Describe()));
	for (const FString& Unit : Fallen)
	{
		BugHunt::Note(BugHunt::Fmt<Lab17Text::Fell>(*Unit));
		Turns->RemoveUnit(Unit);
	}

	TArray<FString> Expected = Units;
	for (const FString& Unit : Fallen)
	{
		Expected.Remove(Unit);
	}
	const FString ExpectedText = FString::Join(Expected, TEXT(" → "));
	BugHunt::Expect(BugHunt::Tr<Lab17Text::Remaining>(), Expected.Num(), Turns->Order.Num());
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab17Text::Matches>(*ExpectedText, *Turns->Describe()), Turns->Order == Expected);
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 쓰러진 유닛은 잘 빠졌는데(개수 PASS) 순서가 틀립니다. 민첩이 가장 낮은 유닛이 어디로 갔나요?
// RemoveUnit 에 중단점을 걸고 RemoveAtSwap 줄 앞뒤로 Order 를 펼쳐 비교하세요.

// [JA] 倒れたユニットはちゃんと抜けた（数は PASS）のに、順番が間違っています。素早さが一番低いユニットはどこへ行きましたか？
// [JA] RemoveUnit にブレークポイントを設定し、RemoveAtSwap の行の前後で Order を展開して比べてください。
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// RemoveAtSwap 은 지울 칸에 "맨 끝 원소"를 옮겨 넣고 배열을 하나 줄입니다. 뒤쪽을 한 칸씩 당기지 않으니 O(1) 이지만,
// 순서가 바뀝니다. 순서가 의미 없는 목록(보이는 적 목록 등)에서만 씁니다. RemoveSwap, RemoveAllSwap 도 같습니다.

// [JA] RemoveAtSwap は消す場所に「最後の要素」を移して、配列を 1 つ縮めます。後ろを 1 つずつ詰めないので O(1) ですが、
// [JA] 順番が変わります。順番に意味のないリスト（見えている敵のリストなど）でだけ使います。RemoveSwap、RemoveAllSwap も同じです。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 순서를 지켜야 하면 RemoveAt(Index) (뒤를 당김) 또는 Order.Remove(Unit).
// 인덱스로 "다음 차례"를 기억하는 코드가 있다면, 지운 칸이 그 인덱스 앞이었는지도 같이 고쳐야 합니다.

// [JA] 順番を守る必要があるなら RemoveAt(Index)（後ろを詰める）または Order.Remove(Unit)。
// [JA] インデックスで「次の番」を覚えているコードがあるなら、消した場所がそのインデックスより前だったかどうかも合わせて直さなければなりません。
#pragma endregion
