#include "Labs/Lab14_WaveRewards.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

void ABH_WaveRewards::ClaimRewards(int32 NumCleared)
{
	for (int32 Wave = 1; Wave <= NumCleared; ++Wave)
	{
		Gold += Rewards[Wave];
		UE_LOG(LogBugHunt, Log, TEXT("  웨이브 %d 보상 -> 골드 %d"), Wave, Gold);
	}
}

void ABH_Lab14::StartLab()
{
	ABH_WaveRewards* Board = SpawnLabActor<ABH_WaveRewards>();
	Board->Rewards = { 10, 20, 30, 40, 50 };
	if (BugHunt::IsHiddenCase())
	{
		Board->Rewards.Reset();
		const int32 NumWaves = BugHunt::CaseInt(0, 3, 9);
		for (int32 i = 0; i < NumWaves; ++i)
		{
			Board->Rewards.Add(BugHunt::CaseInt(0, 5, 99));
		}
	}

	int32 Expected = 0;
	for (int32 Reward : Board->Rewards)
	{
		Expected += Reward;
	}
	BugHunt::Note(FString::Printf(TEXT("웨이브 %d개를 모두 깼다. 보상 합계 %d 골드를 받아야 한다."), Board->Rewards.Num(), Expected));

	Board->ClaimRewards(Board->Rewards.Num());
	BugHunt::Expect(TEXT("받은 골드"), Expected, Board->Gold);
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 디버거가 "Assertion failed: (Index >= 0) & (Index < ArrayNum)" 에서 멈춥니다. Output Log 에는
// "Array index out of bounds: 5 into an array of size 5" 가 남습니다. 숫자 두 개를 적어 두세요.
// 호출 스택에서 TArray::RangeCheck 아래로 내려가 처음 나오는 BugHuntUE.dll! 프레임을 더블클릭하세요.
#pragma endregion

#pragma region 힌트 2
// 지역 창의 Wave 와 Rewards.Num() 을 보세요. 웨이브 번호(1부터)와 배열 인덱스(0부터)가 같은 숫자인가요?
// 로그의 "웨이브 1 보상" 줄에 찍힌 골드는 기획서의 웨이브 1 보상(10)인가요?
#pragma endregion

#pragma region 힌트 3
// 웨이브 번호 Wave 의 보상은 Rewards[Wave - 1] 입니다. 또는 인덱스로 돌고 출력할 때만 +1:
//     for (int32 Index = 0; Index < NumCleared && Index < Rewards.Num(); ++Index) { Gold += Rewards[Index]; }
// 범위가 의심스러우면 Rewards.IsValidIndex(Index) 로 확인합니다.
#pragma endregion
