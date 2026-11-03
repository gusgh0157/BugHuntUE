#include "Labs/Lab35_QuestBoard.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Net/UnrealNetwork.h"

ABH_QuestBoard::ABH_QuestBoard()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ABH_QuestBoard::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABH_QuestBoard, Quests);
}

void ABH_QuestBoard::AddQuest(FName Quest)
{
	FBH_QuestEntry& Entry = Quests.Items.AddDefaulted_GetRef();
	Entry.Quest = Quest;
	Quests.MarkItemDirty(Entry);
}

void ABH_QuestBoard::AddProgress(FName Quest, int32 Amount)
{
	for (FBH_QuestEntry& Entry : Quests.Items)
	{
		if (Entry.Quest == Quest)
		{
			Entry.Progress += Amount;
			UE_LOG(LogBugHunt, Log, TEXT("  %s %s 진행도 -> %d"), *BugHunt::NetTag(this), *Quest.ToString(), Entry.Progress);
			return;
		}
	}
}

int32 ABH_QuestBoard::GetProgress(FName Quest) const
{
	for (const FBH_QuestEntry& Entry : Quests.Items)
	{
		if (Entry.Quest == Quest)
		{
			return Entry.Progress;
		}
	}
	return -1;
}

void ABH_QuestBoard::Multicast_CheckBoard_Implementation(const TArray<FName>& QuestNames, const TArray<int32>& ExpectedProgress)
{
	const FString Tag = BugHunt::NetTag(this);
	for (int32 i = 0; i < QuestNames.Num(); ++i)
	{
		BugHunt::Expect(FString::Printf(TEXT("%s 게시판의 %s 진행도"), *Tag, *QuestNames[i].ToString()), ExpectedProgress[i], GetProgress(QuestNames[i]));
	}
	if (GetNetMode() == NM_Client)
	{
		BugHunt::ExitIfAutomated();
	}
}

// ---------------------------------------------------------------------------
// 아래는 서버에서 도는 시나리오입니다.

void ABH_Lab35::StartLab()
{
	if (GetNetMode() == NM_Client)
	{
		BugHunt::Skip(TEXT("랩 35"), TEXT("서버(리슨 서버 창)에서 실행하세요."));
		FinishLab();
		return;
	}
	if (GetNetMode() == NM_Standalone)
	{
		BugHunt::Note(TEXT("스탠드얼론으로 실행 중. 리슨 서버 + 클라이언트로 실행해야 버그가 보입니다 (README 참고)."));
	}

	const TArray<FName> Names = { TEXT("늑대사냥"), TEXT("약초채집"), TEXT("편지배달") };
	const int32 Wolves = BugHunt::CaseInt(3, 1, 9);
	const int32 Herbs = BugHunt::CaseInt(5, 1, 9);
	WaitForClientThen([this, Names, Wolves, Herbs]
	{
		ABH_QuestBoard* Board = SpawnLabActor<ABH_QuestBoard>(FVector(0.f, 0.f, 100.f));
		for (const FName& Name : Names)
		{
			Board->AddQuest(Name);
		}
		BugHunt::Note(FString::Printf(TEXT("퀘스트 %d개 게시. 늑대 %d마리, 약초 %d개를 차례로 모은다."), Names.Num(), Wolves, Herbs));

		// 게시판이 클라이언트에 도착한 뒤 진행도를 올린다.
		After(0.5f, [Board, Wolves]
		{
			for (int32 i = 0; i < Wolves; ++i)
			{
				Board->AddProgress(TEXT("늑대사냥"), 1);
			}
		});
		After(1.0f, [Board, Herbs] { Board->AddProgress(TEXT("약초채집"), Herbs); });

		// 복제될 시간을 충분히 준 뒤 모든 머신에서 각자 검사한다.
		After(2.5f, [Board, Names, Wolves, Herbs] { Board->Multicast_CheckBoard(Names, { Wolves, Herbs, 0 }); });
		After(3.5f, [this] { FinishLab(); });
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 랩 11 처럼 리슨 서버 + 클라이언트로 실행하세요 (PIE Number of Players 2, Play As Listen Server).
// [리슨 서버] 줄은 통과하고 [클라이언트] 줄만 0 으로 실패합니다. 퀘스트 이름은 클라이언트에 도착했는데 진행도는 왜 안 왔을까요?
// 클라이언트 프로세스에서 ABH_QuestBoard::GetProgress 에 중단점을 걸고 조사식에 Quests.Items 를 펼쳐 보세요.
#pragma endregion

#pragma region 힌트 2
// 일반 UPROPERTY(Replicated) TArray 는 엔진이 매번 이전 값과 비교해서 바뀐 걸 찾아냅니다.
// FFastArraySerializer 는 비교하지 않습니다. "이 원소가 바뀌었다" 는 표시(ReplicationKey)를 보고 그 원소만 보냅니다.
// AddQuest 와 AddProgress 를 나란히 놓고 보세요. 하나에만 있는 줄이 있습니다.
#pragma endregion

#pragma region 힌트 3
// 원소를 바꾼 뒤에는 그 원소를 표시해야 합니다:
//     Entry.Progress += Amount;
//     Quests.MarkItemDirty(Entry);
// 원소를 지웠다면 Quests.MarkArrayDirty(). 클라이언트에서 원소별로 반응하려면 FBH_QuestEntry 에
// PostReplicatedChange(const FBH_QuestList&) 같은 콜백을 만들면 됩니다 (RepNotify 의 원소 버전).
#pragma endregion
