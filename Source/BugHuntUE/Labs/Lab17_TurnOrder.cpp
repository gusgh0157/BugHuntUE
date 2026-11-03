#include "Labs/Lab17_TurnOrder.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

void ABH_TurnOrder::RemoveUnit(const FString& Unit)
{
	const int32 Index = Order.IndexOfByKey(Unit);
	if (Index != INDEX_NONE)
	{
		Order.RemoveAtSwap(Index);   // 가운데를 빼도 뒤를 당기지 않아서 빠르다
	}
}

FString ABH_TurnOrder::Describe() const
{
	return FString::Join(Order, TEXT(" → "));
}

void ABH_Lab17::StartLab()
{
	ABH_TurnOrder* Turns = SpawnLabActor<ABH_TurnOrder>();
	TArray<FString> Units = { TEXT("도적"), TEXT("궁수"), TEXT("기사"), TEXT("마법사"), TEXT("사제") };
	TArray<FString> Fallen = { TEXT("궁수") };
	if (BugHunt::IsHiddenCase())
	{
		const TCHAR* Pool[] = { TEXT("도적"), TEXT("궁수"), TEXT("기사"), TEXT("마법사"), TEXT("사제"), TEXT("창병"), TEXT("검사"), TEXT("드루이드") };
		Units.Reset();
		const int32 Count = BugHunt::CaseInt(0, 5, UE_ARRAY_COUNT(Pool));
		for (int32 i = 0; i < Count; ++i)
		{
			Units.Add(Pool[i]);
		}
		// 맨 끝이 아닌 유닛 두 명이 쓰러진다
		Fallen = { Units[BugHunt::CaseInt(0, 0, 1)], Units[BugHunt::CaseInt(0, 2, Count - 2)] };
	}

	Turns->Order = Units;
	BugHunt::Note(FString::Printf(TEXT("행동 순서 (민첩 순): %s"), *Turns->Describe()));
	for (const FString& Unit : Fallen)
	{
		BugHunt::Note(FString::Printf(TEXT("%s 쓰러짐"), *Unit));
		Turns->RemoveUnit(Unit);
	}

	TArray<FString> Expected = Units;
	for (const FString& Unit : Fallen)
	{
		Expected.Remove(Unit);
	}
	const FString ExpectedText = FString::Join(Expected, TEXT(" → "));
	BugHunt::Expect(TEXT("남은 유닛 수"), Expected.Num(), Turns->Order.Num());
	BugHunt::ExpectTrue(FString::Printf(TEXT("행동 순서 == %s (실제: %s)"), *ExpectedText, *Turns->Describe()), Turns->Order == Expected);
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 쓰러진 유닛은 잘 빠졌는데(개수 PASS) 순서가 틀립니다. 민첩이 가장 낮은 유닛이 어디로 갔나요?
// RemoveUnit 에 중단점을 걸고 RemoveAtSwap 줄 앞뒤로 Order 를 펼쳐 비교하세요.
#pragma endregion

#pragma region 힌트 2
// RemoveAtSwap 은 지울 칸에 "맨 끝 원소"를 옮겨 넣고 배열을 하나 줄입니다. 뒤쪽을 한 칸씩 당기지 않으니 O(1) 이지만,
// 순서가 바뀝니다. 순서가 의미 없는 목록(보이는 적 목록 등)에서만 씁니다. RemoveSwap, RemoveAllSwap 도 같습니다.
#pragma endregion

#pragma region 힌트 3
// 순서를 지켜야 하면 RemoveAt(Index) (뒤를 당김) 또는 Order.Remove(Unit).
// 인덱스로 "다음 차례"를 기억하는 코드가 있다면, 지운 칸이 그 인덱스 앞이었는지도 같이 고쳐야 합니다.
#pragma endregion
