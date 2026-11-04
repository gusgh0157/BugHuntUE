#include "Labs/Lab49_Pathfinding.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntHang.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab49Text
{
BH_TEXT(FindPathHang, "길찾기 %s → %s", "経路探索 %s → %s");
BH_TEXT(NoPath,       "(경로 없음)", "（経路なし）");
BH_TEXT(Gate,         "성문", "城門");
BH_TEXT(Plaza,        "광장", "広場");
BH_TEXT(Market,       "시장", "市場");
BH_TEXT(Harbor,       "항구", "港");
BH_TEXT(Inn,          "여관", "宿屋");
BH_TEXT(Smithy,       "대장간", "鍛冶屋");
BH_TEXT(Lighthouse,   "등대", "灯台");
BH_TEXT(Cliff,        "절벽", "崖");
BH_TEXT(Alley,        "골목%d", "路地%d");
BH_TEXT(Plan,         "지점 %d개. 성문 → %s, 그리고 성문 → 등대 (강 건너, 다리가 끊김).", "地点 %d 個。城門 → %s、そして城門 → 灯台（川の向こう、橋が途切れている）。");
BH_TEXT(PathLog,      "경로: %s", "経路: %s");
BH_TEXT(HopCount,     "성문 → %s 경로의 길 수", "城門 → %s の経路の道の数");
BH_TEXT(Unreachable,  "성문 → 등대 경로의 지점 수 (못 가면 0)", "城門 → 灯台の経路の地点数（行けなければ 0）");
}

int32 FBH_RoadGraph::AddNode(const FString& Name)
{
	FNode& Node = Nodes.AddDefaulted_GetRef();
	Node.Name = Name;
	return Nodes.Num() - 1;
}

void FBH_RoadGraph::Connect(int32 A, int32 B)
{
	Nodes[A].Links.AddUnique(B);
	Nodes[B].Links.AddUnique(A);
}

TArray<int32> FBH_RoadGraph::FindPath(int32 From, int32 To) const
{
	TArray<int32> Frontier = { From };
	TMap<int32, int32> CameFrom = { { From, INDEX_NONE } };

	BugHunt::FHangGuard Guard(BugHunt::Fmt<Lab49Text::FindPathHang>(*Nodes[From].Name, *Nodes[To].Name), 2000000);
	bool bFound = false;
	while (Frontier.Num() > 0)
	{
		BH_HANG_CHECK(Guard);   // 실습용 안전장치 (Core/BugHuntHang.h). 버그는 이 줄이 아니다.
		// [JA] 実習用の安全装置（Core/BugHuntHang.h）。バグはこの行ではない。

		const int32 Current = Frontier[0];
		Frontier.RemoveAt(0);
		if (Current == To)
		{
			bFound = true;
			break;
		}

		TSet<int32> Visited;
		Visited.Add(Current);
		for (int32 Next : Nodes[Current].Links)
		{
			if (!Visited.Contains(Next))
			{
				Visited.Add(Next);
				CameFrom.FindOrAdd(Next, Current);
				Frontier.Add(Next);
			}
		}
	}

	TArray<int32> Path;
	if (bFound)
	{
		for (int32 Node = To; Node != INDEX_NONE && Path.Num() <= Nodes.Num(); Node = CameFrom[Node])
		{
			Path.Insert(Node, 0);
		}
	}
	return Path;
}

namespace
{
FString PathToString(const FBH_RoadGraph& Graph, const TArray<int32>& Path)
{
	TArray<FString> Names;
	for (int32 Node : Path)
	{
		Names.Add(Graph.Nodes[Node].Name);
	}
	return Names.Num() > 0 ? FString::Join(Names, TEXT(" → ")) : BugHunt::Tr<Lab49Text::NoPath>();
}
}

void ABH_Lab49::StartLab()
{
	// 공개 입력: 광장을 가운데 둔 마을 + 강 건너 등대 (다리 끊김)
	//   성문 - 광장 - 시장 - 항구
	//           |  \    |
	//         여관 - 대장간
	//   (강 건너) 등대 - 절벽
	// [JA] 公開入力: 広場を中心にした町 + 川の向こうの灯台（橋が途切れている）
	// [JA]   城門 - 広場 - 市場 - 港
	// [JA]           |  \    |
	// [JA]         宿屋 - 鍛冶屋
	// [JA]   （川の向こう）灯台 - 崖
	FBH_RoadGraph Graph;
	const int32 Gate = Graph.AddNode(BugHunt::Tr<Lab49Text::Gate>());
	const int32 Plaza = Graph.AddNode(BugHunt::Tr<Lab49Text::Plaza>());
	const int32 Market = Graph.AddNode(BugHunt::Tr<Lab49Text::Market>());
	const int32 Harbor = Graph.AddNode(BugHunt::Tr<Lab49Text::Harbor>());
	const int32 Inn = Graph.AddNode(BugHunt::Tr<Lab49Text::Inn>());
	const int32 Smithy = Graph.AddNode(BugHunt::Tr<Lab49Text::Smithy>());
	const int32 Lighthouse = Graph.AddNode(BugHunt::Tr<Lab49Text::Lighthouse>());
	const int32 Cliff = Graph.AddNode(BugHunt::Tr<Lab49Text::Cliff>());
	Graph.Connect(Gate, Plaza);
	Graph.Connect(Plaza, Market);
	Graph.Connect(Market, Harbor);
	Graph.Connect(Plaza, Inn);
	Graph.Connect(Plaza, Smithy);
	Graph.Connect(Inn, Smithy);
	Graph.Connect(Market, Smithy);
	Graph.Connect(Lighthouse, Cliff);

	// 숨은 입력: 마을 쪽에 지점과 길을 더 붙인다 (모두 성문과 이어지고 강 건너와는 끊겨 있다)
	// [JA] 隠し入力: 町の側に地点と道をさらに付け足す（すべて城門とつながり、川の向こうとは途切れている）
	const int32 ExtraNodes = BugHunt::CaseInt(0, 2, 6);
	for (int32 i = 0; i < ExtraNodes; ++i)
	{
		const int32 New = Graph.AddNode(BugHunt::Fmt<Lab49Text::Alley>(i + 1));
		const int32 TownNodes[] = { Gate, Plaza, Market, Harbor, Inn, Smithy };
		Graph.Connect(New, TownNodes[BugHunt::CaseInt(0, 0, UE_ARRAY_COUNT(TownNodes) - 1)]);
		Graph.Connect(New, TownNodes[BugHunt::CaseInt(0, 0, UE_ARRAY_COUNT(TownNodes) - 1)]);
	}
	const int32 Dest = BugHunt::IsHiddenCase() ? Graph.Nodes.Num() - 1 : Harbor;

	// 기대값: 너비 우선 탐색은 지나는 길 수가 가장 적은 경로를 찾는다. 이 길 수를 따로 센다 (버그 없음).
	// [JA] 期待値: 幅優先探索は、通過する道の数が最も少ない経路を見つける。この道の数を別に数える（バグなし）。
	TArray<int32> Hops;
	Hops.Init(INDEX_NONE, Graph.Nodes.Num());
	Hops[Gate] = 0;
	for (bool bChanged = true; bChanged;)
	{
		bChanged = false;
		for (int32 Node = 0; Node < Graph.Nodes.Num(); ++Node)
		{
			for (int32 Next : Graph.Nodes[Node].Links)
			{
				if (Hops[Node] != INDEX_NONE && (Hops[Next] == INDEX_NONE || Hops[Next] > Hops[Node] + 1))
				{
					Hops[Next] = Hops[Node] + 1;
					bChanged = true;
				}
			}
		}
	}

	BugHunt::Note(BugHunt::Fmt<Lab49Text::Plan>(Graph.Nodes.Num(), *Graph.Nodes[Dest].Name));

	const TArray<int32> ToDest = Graph.FindPath(Gate, Dest);
	BugHunt::Note(BugHunt::Fmt<Lab49Text::PathLog>(*PathToString(Graph, ToDest)));
	BugHunt::Expect(BugHunt::Fmt<Lab49Text::HopCount>(*Graph.Nodes[Dest].Name), Hops[Dest], ToDest.Num() - 1);

	const TArray<int32> ToLighthouse = Graph.FindPath(Gate, Lighthouse);
	BugHunt::Expect(BugHunt::Tr<Lab49Text::Unreachable>(), 0, ToLighthouse.Num());
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 성문 → 항구는 잘 찾는데 성문 → 등대에서 "응답 없음" 입니다. 못 가는 곳을 물으면 끝나지 않는 것입니다.
// BugHunt.HangBudgetSeconds 30 으로 굳는 시간을 늘리고 모두 중단(Ctrl+Alt+Break) → 스레드 창에서 GameThread.
// FindPath 프레임에서 조사식에 Frontier.Num(), CameFrom.Num(), Visited.Num() 을 넣고 F5 → 모두 중단을 몇 번 반복하세요.
// 지점은 8개뿐인데 Frontier 는 몇 개인가요?

// [JA] 城門 → 港はちゃんと見つかるのに、城門 → 灯台では「応答なし」です。行けない場所を問い合わせると終わらないのです。
// [JA] BugHunt.HangBudgetSeconds 30 で固まる時間を延ばし、すべて中断（Ctrl+Alt+Break）→ スレッド ウィンドウで GameThread。
// [JA] FindPath のフレームで、ウォッチに Frontier.Num(), CameFrom.Num(), Visited.Num() を入れて F5 → すべて中断を何度か繰り返してください。
// [JA] 地点は 8 個しかないのに、Frontier はいくつありますか？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 너비 우선 탐색이 끝나는 이유는 "한 번 넣은 지점은 다시 넣지 않기" 때문입니다.
// 그 기억(Visited)이 루프 한 바퀴만큼만 살아 있다면? 광장 → 여관 → 광장 → 여관 ... 을 영원히 다시 넣습니다.
// 갈 수 있는 곳은 우연히 도착해서 멈추지만 (그래서 성문 → 항구는 통과), 갈 수 없는 곳은 큐가 끝없이 불어납니다.
// 지역 변수의 수명(어느 중괄호 안에서 만들어지는가)을 보세요.

// [JA] 幅優先探索が終わるのは、「一度入れた地点は二度と入れない」からです。
// [JA] その記憶（Visited）がループ 1 周分しか生きていなかったら？ 広場 → 宿屋 → 広場 → 宿屋 ... を永遠に入れ直します。
// [JA] 行ける場所はたまたま到着して止まりますが（だから城門 → 港は合格）、行けない場所ではキューが際限なく膨らみます。
// [JA] ローカル変数の寿命（どの中括弧の中で作られるか）を見てください。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// Visited 를 while 바깥, 탐색 전체에 하나만 두고 From 을 미리 넣습니다.
//     TSet<int32> Visited = { From };
//     while (...) { ... if (!Visited.Contains(Next)) { Visited.Add(Next); ... } }
// 사실 CameFrom 이 이미 "방문한 지점" 목록입니다. CameFrom.Contains(Next) 로 검사해도 됩니다.

// [JA] Visited を while の外に、探索全体で 1 つだけ置き、From をあらかじめ入れておきます。
// [JA]     TSet<int32> Visited = { From };
// [JA]     while (...) { ... if (!Visited.Contains(Next)) { Visited.Add(Next); ... } }
// [JA] 実は CameFrom がすでに「訪問した地点」の一覧です。CameFrom.Contains(Next) で調べてもかまいません。
#pragma endregion
