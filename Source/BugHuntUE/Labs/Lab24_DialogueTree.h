#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab24_DialogueTree.generated.h"

// 대화 노드. UObject 가 아닌 보통 C++ 객체이고 TSharedPtr 로 수명을 관리한다. (GC 와 상관없음)
// [JA] 会話ノード。UObject ではない普通の C++ オブジェクトで、TSharedPtr で寿命を管理する。（GC とは無関係）
struct FBH_DialogueNode
{
	explicit FBH_DialogueNode(const FString& InLine);
	~FBH_DialogueNode();

	FString Line;
	TArray<TSharedPtr<FBH_DialogueNode>> Choices;
	TSharedPtr<FBH_DialogueNode> Parent;     // "이전으로" 버튼용
	// [JA] 「前へ」ボタン用

	// 지금 메모리에 살아 있는 노드 수 (생성자에서 +1, 소멸자에서 -1)
	// [JA] 今メモリに生きているノード数（コンストラクターで +1、デストラクターで -1）
	static int32 NumAlive;
};

UCLASS(NotPlaceable)
class ABH_Npc : public AActor
{
	GENERATED_BODY()

public:
	// 대화 트리를 만든다: 인사 → 선택지 NumChoices 개 → 각 선택지마다 대답 하나
	// [JA] 会話ツリーを作る: あいさつ → 選択肢 NumChoices 個 → 選択肢ごとに返答 1 つ
	void StartConversation(int32 NumChoices);
	void EndConversation();

	// 대화 진행: 선택지 고르기 / 이전으로
	// [JA] 会話の進行: 選択肢を選ぶ / 前へ
	void Choose(int32 Index);
	void GoBack();

	FString CurrentLine() const;

private:
	TSharedPtr<FBH_DialogueNode> Root;
	TSharedPtr<FBH_DialogueNode> Current;
};

UCLASS(NotPlaceable)
class ABH_Lab24 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
