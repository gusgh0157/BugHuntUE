#include "Labs/Lab24_DialogueTree.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

int32 FBH_DialogueNode::NumAlive = 0;

FBH_DialogueNode::FBH_DialogueNode(const FString& InLine)
	: Line(InLine)
{
	++NumAlive;
}

FBH_DialogueNode::~FBH_DialogueNode()
{
	--NumAlive;
}

namespace
{
TSharedPtr<FBH_DialogueNode> AddChoice(const TSharedPtr<FBH_DialogueNode>& Parent, const FString& Line)
{
	TSharedPtr<FBH_DialogueNode> Child = MakeShared<FBH_DialogueNode>(Line);
	Child->Parent = Parent;
	Parent->Choices.Add(Child);
	return Child;
}
}

void ABH_Npc::StartConversation(int32 NumChoices)
{
	Root = MakeShared<FBH_DialogueNode>(TEXT("어서 오게, 여행자."));
	for (int32 i = 0; i < NumChoices; ++i)
	{
		TSharedPtr<FBH_DialogueNode> Choice = AddChoice(Root, FString::Printf(TEXT("질문 %d"), i + 1));
		AddChoice(Choice, FString::Printf(TEXT("대답 %d"), i + 1));
	}
	Current = Root;
}

void ABH_Npc::EndConversation()
{
	Current.Reset();
	Root.Reset();
}

void ABH_Npc::Choose(int32 Index)
{
	if (Current && Current->Choices.IsValidIndex(Index))
	{
		Current = Current->Choices[Index];
	}
}

void ABH_Npc::GoBack()
{
	if (Current && Current->Parent)
	{
		Current = Current->Parent;
	}
}

FString ABH_Npc::CurrentLine() const
{
	return Current ? Current->Line : FString();
}

void ABH_Lab24::StartLab()
{
	const int32 NumChoices = BugHunt::CaseInt(3, 2, 7);
	const int32 Picked = BugHunt::CaseInt(1, 0, NumChoices - 1);
	const int32 AliveBefore = FBH_DialogueNode::NumAlive;

	ABH_Npc* Npc = SpawnLabActor<ABH_Npc>();
	Npc->StartConversation(NumChoices);
	BugHunt::Note(FString::Printf(TEXT("대화 노드 %d개를 만들었다. 질문 %d → 대답 → 이전으로 두 번 → 대화 끝."),
		FBH_DialogueNode::NumAlive - AliveBefore, Picked + 1));

	Npc->Choose(Picked);
	Npc->Choose(0);
	const FString Answer = Npc->CurrentLine();
	Npc->GoBack();
	Npc->GoBack();
	const FString BackToStart = Npc->CurrentLine();
	Npc->EndConversation();

	BugHunt::ExpectTrue(FString::Printf(TEXT("고른 질문의 대답 == \"대답 %d\" (실제: \"%s\")"), Picked + 1, *Answer),
		Answer == FString::Printf(TEXT("대답 %d"), Picked + 1));
	BugHunt::ExpectTrue(FString::Printf(TEXT("이전으로 두 번 == 첫 인사 (실제: \"%s\")"), *BackToStart),
		BackToStart == TEXT("어서 오게, 여행자."));
	BugHunt::Expect(TEXT("대화가 끝난 뒤 메모리에 남은 대화 노드"), 0, FBH_DialogueNode::NumAlive - AliveBefore);
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// EndConversation 이 Root 와 Current 를 둘 다 Reset 했는데 노드가 하나도 안 지워졌습니다.
// FBH_DialogueNode 소멸자에 중단점을 걸고 랩을 돌려 보세요. 몇 번 멈추나요?
// EndConversation 의 Root.Reset() 줄에서 멈춘 뒤 조사식에 Root 를 넣어 펼쳐 보세요.
// TSharedPtr 의 참조 수(SharedReferenceCount)가 보입니다. 아무도 안 쓸 텐데 왜 1이 아닐까요?
#pragma endregion

#pragma region 힌트 2
// 루트의 참조 수를 세어 보세요: Root 멤버 1, Current 1(대화 중), 그리고 각 선택지의 Parent 가 하나씩.
// Root 가 Reset 되어도 자식들의 Parent 가 루트를 붙잡고, 루트의 Choices 가 자식들을 붙잡습니다.
// 서로가 서로를 붙잡는 고리(순환 참조)는 참조 수가 절대 0 이 되지 않습니다. 크래시도 경고도 없이 메모리만 샙니다.
// (GC 가 있는 UObject 라면 고리째 수거되지만, TSharedPtr 는 참조 수만 봅니다.)
#pragma endregion

#pragma region 힌트 3
// 소유하지 않는 쪽(자식 → 부모)은 TWeakPtr 로 바꾸세요.
//     TWeakPtr<FBH_DialogueNode> Parent;
// 쓸 때는 Pin() 으로 잠깐 TSharedPtr 를 얻습니다:
//     if (TSharedPtr<FBH_DialogueNode> Up = Current->Parent.Pin()) { Current = Up; }
// 규칙: 소유 방향(부모 → 자식)은 TSharedPtr/TSharedRef, 거꾸로 가리키는 쪽은 TWeakPtr.
#pragma endregion
