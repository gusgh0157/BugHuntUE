#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab25_DialogueTree.generated.h"

// 대화 노드. UObject 가 아닌 보통 C++ 객체이고 TSharedPtr 로 수명을 관리한다. (GC 와 상관없음)
struct FBH_DialogueNode
{
	explicit FBH_DialogueNode(const FString& InLine);
	~FBH_DialogueNode();

	FString Line;
	TArray<TSharedPtr<FBH_DialogueNode>> Choices;
	TSharedPtr<FBH_DialogueNode> Parent;     // "이전으로" 버튼용

	// 지금 메모리에 살아 있는 노드 수 (생성자에서 +1, 소멸자에서 -1)
	static int32 NumAlive;
};

UCLASS(NotPlaceable)
class ABH_Npc : public AActor
{
	GENERATED_BODY()

public:
	// 대화 트리를 만든다: 인사 → 선택지 NumChoices 개 → 각 선택지마다 대답 하나
	void StartConversation(int32 NumChoices);
	void EndConversation();

	// 대화 진행: 선택지 고르기 / 이전으로
	void Choose(int32 Index);
	void GoBack();

	FString CurrentLine() const;

private:
	TSharedPtr<FBH_DialogueNode> Root;
	TSharedPtr<FBH_DialogueNode> Current;
};

UCLASS(NotPlaceable)
class ABH_Lab25 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
