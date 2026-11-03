#include "Labs/Lab04_QuestLog.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Engine/Engine.h"

void UBH_QuestLog::AddEntry(const FString& Quest)
{
	Entries.Add(Quest);
	UE_LOG(LogBugHunt, Log, TEXT("  퀘스트 기록: %s (총 %d개)"), *Quest, Entries.Num());
}

void ABH_QuestGiver::BeginPlay()
{
	Super::BeginPlay();
	QuestLog = NewObject<UBH_QuestLog>(this, TEXT("QuestLog"));
}

void ABH_QuestGiver::AcceptQuest(const FString& Quest)
{
	QuestLog->AddEntry(Quest);
}

int32 ABH_QuestGiver::NumAccepted() const
{
	return QuestLog->NumEntries();
}

void ABH_Lab04::StartLab()
{
	Giver = SpawnLabActor<ABH_QuestGiver>();
	Giver->AcceptQuest(TEXT("늑대 다섯 마리 사냥"));

	// 실제 게임에서는 GC가 약 60초마다 돈다. 1분을 기다리지 않도록 지금 바로 GC를 돌린다.
	// (콘솔 명령 `obj gc` 와 같다.)
	BugHunt::Note(TEXT("... 시간이 흐른다 (GC 실행) ..."));
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	Giver->AcceptQuest(TEXT("대장장이에게 광석 전달"));
	BugHunt::Expect(TEXT("받은 퀘스트 수"), 2, Giver->NumAccepted());
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 이 랩은 "해제된 메모리 사용"이라 증상이 매번 다를 수 있습니다:
// 읽기/쓰기/실행 액세스 위반, 호출 스택 맨 위에 이상한 주소(0x00000000xxxxxxxx), 또는 조용히 틀린 숫자.
// 엉뚱한 곳에서 터지면 호출 스택에서 ABH_ 로 시작하는 첫 줄로 가세요. AcceptQuest 의 QuestLog 를
// 지역 창에서 펼치면 Entries 나 이름(NamePrivate)이 정상인가요?
#pragma endregion

#pragma region 힌트 2
// 같은 객체를 GC 전과 후에 비교하세요. StartLab 의 CollectGarbage 줄에 중단점 → 조사식에
//     Giver->QuestLog
// 를 넣고 주소와 내용을 적어 둡니다. F10 으로 GC를 넘긴 뒤 다시 보세요.
// 실제 게임에서 재현하려면 콘솔에 gc.CollectGarbageEveryFrame 1 을 넣으면 매 프레임 GC가 돌아
// 이런 버그가 바로 드러납니다.
#pragma endregion

#pragma region 힌트 3
// GC는 "UPROPERTY 로 이어진 참조"만 따라가서 살아 있는 객체를 찾습니다.
// Outer(NewObject 의 첫 인자)는 객체를 살려 두지 않습니다. 헤더에서 QuestLog 선언을 다시 보세요.
// 증상을 확실하게 하려면 명령줄에 -stomp2malloc 을 붙여 실행하세요. 해제된 메모리를 건드리는 순간 바로 멈춥니다.
#pragma endregion
