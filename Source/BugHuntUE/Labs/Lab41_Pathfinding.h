#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab41_Pathfinding.generated.h"

// 마을 길 그래프. 지점(노드)과 양방향 길(링크). 다리가 끊기면 섬처럼 떨어진 지점이 생긴다.
struct FBH_RoadGraph
{
	struct FNode
	{
		FString Name;
		TArray<int32> Links;
	};

	int32 AddNode(const FString& Name);
	void Connect(int32 A, int32 B);

	// 너비 우선 탐색으로 가장 짧은 경로(지나는 지점 목록, From 과 To 포함)를 찾는다. 못 가면 빈 배열.
	TArray<int32> FindPath(int32 From, int32 To) const;

	TArray<FNode> Nodes;
};

UCLASS(NotPlaceable)
class ABH_Lab41 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
