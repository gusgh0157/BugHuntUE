#include "Labs/Lab31_BountyBoard.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

void ABH_BountyBoard::RecordHunt(FName Target, const TArray<FName>& Killed)
{
	// 의뢰 대상은 거의 매번 잡으니까 칸을 미리 찾아 둔다.
	int32* TargetKills = Kills.Find(Target);

	for (const FName& Monster : Killed)
	{
		if (Monster == Target)
		{
			++(*TargetKills);
		}
		else
		{
			++Kills.FindOrAdd(Monster);
		}
		UE_LOG(LogBugHunt, Log, TEXT("  %s 사냥 (종류 %d개, 의뢰 대상 %d마리)"), *Monster.ToString(), Kills.Num(), *TargetKills);
	}
}

int32 ABH_BountyBoard::KillsOf(FName Monster) const
{
	return Kills.FindRef(Monster);
}

void ABH_Lab31::StartLab()
{
	const FName Wolf(TEXT("Wolf"));
	ABH_BountyBoard* Board = SpawnLabActor<ABH_BountyBoard>();
	Board->Kills.Add(Wolf, 0);
	Board->Kills.Add(TEXT("Bat"), 0);
	Board->Kills.Add(TEXT("Boar"), 0);

	// 공개 입력: 늑대, 슬라임(새 종류), 늑대, 고블린(새 종류 → 5종류), 늑대
	TArray<FName> Killed = { Wolf, TEXT("Slime"), Wolf, TEXT("Goblin"), Wolf };
	if (BugHunt::IsHiddenCase())
	{
		const FName Known[] = { TEXT("Bat"), TEXT("Boar") };
		Killed.Reset();
		auto AddSome = [&Killed, &Known, Wolf](int32 Wolves)
		{
			for (int32 i = 0; i < Wolves; ++i)
			{
				Killed.Add(Wolf);
				if (BugHunt::CaseInt(0, 0, 1) == 1)
				{
					Killed.Add(Known[BugHunt::CaseInt(0, 0, 1)]);
				}
			}
		};
		AddSome(BugHunt::CaseInt(0, 1, 3));
		Killed.Add(TEXT("Slime"));
		AddSome(BugHunt::CaseInt(0, 0, 2));
		Killed.Add(TEXT("Goblin"));
		AddSome(BugHunt::CaseInt(0, 1, 3));
	}

	int32 ExpectedWolves = 0;
	for (const FName& Monster : Killed)
	{
		ExpectedWolves += Monster == Wolf ? 1 : 0;
	}
	BugHunt::Note(FString::Printf(TEXT("의뢰 대상: 늑대. 사냥 %d번 중 늑대 %d마리."), Killed.Num(), ExpectedWolves));

	Board->RecordHunt(Wolf, Killed);
	BugHunt::Expect(TEXT("게시판의 늑대 사냥 수"), ExpectedWolves, Board->KillsOf(Wolf));
	BugHunt::Expect(TEXT("슬라임 사냥 수"), 1, Board->KillsOf(TEXT("Slime")));
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 로그의 "의뢰 대상 N마리" 는 맞게 올라가는데 게시판 값은 다릅니다. 둘은 같은 칸이어야 합니다.
// RecordHunt 에 중단점을 걸고 조사식에 TargetKills 와 Kills.Find(Target) 를 넣어 비교하세요. 언제부터 달라지나요?
#pragma endregion

#pragma region 힌트 2
// TMap 은 내부적으로 원소를 배열(TSparseArray)에 담습니다. 새 키를 넣다가 칸이 모자라면 원소를 새 메모리로 옮깁니다.
// Find 가 돌려준 포인터는 "다음 Add/FindOrAdd/Emplace/Remove 전까지만" 유효합니다. 랩 24 의 TArray 와 같은 이야기입니다.
// 이 게시판은 인라인 버퍼라 옛 칸에 써도 죽지 않고 값만 사라집니다. 기본 할당자였다면 해제된 힙에 썼을 것입니다.
#pragma endregion

#pragma region 힌트 3
// 맵을 바꿀 수 있는 호출 뒤에는 다시 찾으세요. 가장 간단한 방법:
//     ++Kills.FindOrAdd(Monster);   // 대상이든 아니든 같은 방식으로
// 의뢰 대상만 따로 세고 싶다면 포인터 대신 지역 int32 에 세었다가 끝에 Kills[Target] += Count.
#pragma endregion
