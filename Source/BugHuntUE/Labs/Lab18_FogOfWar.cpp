#include "Labs/Lab18_FogOfWar.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

void ABH_FogOfWar::Reveal(int32 X, int32 Y, const FString& Source)
{
	Revealed.Add({ X, Y, Source });
}

bool ABH_FogOfWar::IsRevealed(int32 X, int32 Y) const
{
	return Revealed.Contains({ X, Y, FString() });
}

void ABH_Lab18::StartLab()
{
	ABH_FogOfWar* Fog = SpawnLabActor<ABH_FogOfWar>();

	// 플레이어는 (0,0)~(3,0) 을 걷고, 정찰병은 (2,0)~(2,3) 을 걷는다. 겹치는 칸은 (2,0) 하나.
	const int32 Length = BugHunt::CaseInt(4, 3, 9);
	const int32 Cross = BugHunt::CaseInt(2, 0, Length - 1);
	TSet<FIntPoint> Unique;
	for (int32 i = 0; i < Length; ++i)
	{
		Fog->Reveal(i, 0, TEXT("플레이어"));
		Unique.Add(FIntPoint(i, 0));
	}
	for (int32 j = 0; j < Length; ++j)
	{
		Fog->Reveal(Cross, j, TEXT("정찰병"));
		Unique.Add(FIntPoint(Cross, j));
	}
	BugHunt::Note(FString::Printf(TEXT("플레이어와 정찰병이 칸 %d개씩 밝혔다. 겹치는 칸 (%d,0)."), Length, Cross));

	BugHunt::Expect(TEXT("밝혀진 칸 수 (겹친 칸은 한 번)"), Unique.Num(), Fog->Revealed.Num());
	// 밝힌 칸 하나하나를 다시 물어본다 (칸 하나만 보면 운 좋게 같은 버킷에 들어가 통과할 수 있다).
	int32 Found = 0;
	for (int32 i = 0; i < Length; ++i)
	{
		Found += Fog->IsRevealed(i, 0) ? 1 : 0;
		Found += Fog->IsRevealed(Cross, i) ? 1 : 0;
	}
	BugHunt::Expect(TEXT("밝힌 칸을 물었을 때 밝혀졌다고 답한 수"), 2 * Length, Found);
	BugHunt::ExpectTrue(TEXT("(9,9) 는 아직 안개"), !Fog->IsRevealed(9, 9));
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 밝힌 칸인데 IsRevealed 가 false 이고, 겹친 칸이 두 번 들어 있습니다.
// 조사식에 Revealed 를 넣고 펼쳐 겹친 칸 두 개의 X, Y, Source 를 보세요. operator== 로 비교하면 둘은 같은가요?
// 그런데 왜 TSet 은 둘 다 넣었을까요? TSet 이 원소를 찾을 때 operator== 보다 먼저 보는 것은?
#pragma endregion

#pragma region 힌트 2
// TSet/TMap 은 GetTypeHash 로 칸(버킷)을 고른 뒤, 그 칸 안에서만 operator== 로 비교합니다.
// "a == b 이면 GetTypeHash(a) == GetTypeHash(b)" 가 반드시 성립해야 합니다. 여기서는 == 이 보지 않는 Source 가 해시에 섞여서,
// 같은 칸인데 다른 버킷을 찾아갑니다. (운 나쁘게 같은 버킷이면 찾기도 하니 결과가 들쭉날쭉합니다.)
#pragma endregion

#pragma region 힌트 3
// 해시는 == 에 쓰는 필드만으로 만듭니다.
//     return HashCombine(GetTypeHash(Key.X), GetTypeHash(Key.Y));
// 같은 함정: 키를 넣은 뒤에 키의 필드를 바꾸면(예: TSet 원소를 const_cast 로 고침) 그 원소는 엉뚱한 버킷에 남습니다.
// 키는 바꾸지 말고 Remove 한 뒤 새로 Add 하세요.
#pragma endregion
