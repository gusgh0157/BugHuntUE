#include "Labs/Lab42_GuildVault.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Async/ParallelFor.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab42Text
{
BH_TEXT(Quests, "길드원 %d명이 동시에 퀘스트를 끝내고 보상을 금고에 넣는다. 합계 %d 골드.", "ギルドメンバー %d 人が同時にクエストを終え、報酬を金庫に入れる。合計 %d ゴールド。");
BH_TEXT(VaultGold, "금고의 골드", "金庫のゴールド");
}

void FBH_GuildVault::Deposit(int32 Amount)
{
	// Gold += Amount 를 풀어 쓴 것. CPU 도 실제로 이렇게 읽고, 더하고, 쓴다.
	// [JA] Gold += Amount を展開して書いたもの。CPU も実際にこのように読み、足し、書く。
	const int32 Before = Gold;
	BugHunt::RaceWindow();          // 실습용 확대경 (Core/BugHuntProbe.h). 지우지 마세요
	// [JA] 実習用の拡大鏡（Core/BugHuntProbe.h）。消さないでください
	Gold = Before + Amount;
}

void ABH_Lab42::StartLab()
{
	FBH_GuildVault Vault;
	const int32 Count = BugHunt::CaseInt(4, 2, 8);
	TArray<int32> Rewards;
	int32 Expected = 0;
	for (int32 i = 0; i < Count; ++i)
	{
		Rewards.Add(BugHunt::CaseInt(100 * (i + 1), 10, 500));
		Expected += Rewards.Last();
	}
	BugHunt::Note(BugHunt::Fmt<Lab42Text::Quests>(Count, Expected));

	// 보상 처리는 작업 스레드들에서 동시에 돈다 (게임 스레드는 끝날 때까지 기다린다).
	// [JA] 報酬の処理はワーカースレッドで同時に走る（ゲームスレッドは終わるまで待つ）。
	ParallelFor(Count, [&Vault, &Rewards](int32 Index)
	{
		Vault.Deposit(Rewards[Index]);
	}, EParallelForFlags::Unbalanced);

	BugHunt::Expect(BugHunt::Tr<Lab42Text::VaultGold>(), Expected, Vault.Gold);
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 입금은 모두 했는데 골드가 모자랍니다.
// Deposit 에 추적점 "{FPlatformTLS::GetCurrentThreadId()} 이전 {Before} 입금 {Amount}" 을 걸어 보세요. 같은 Before 를 읽은 스레드가 있나요?
// 또는 Gold = Before + Amount 줄에 중단점 → 디버그 → 창 → 스레드(Ctrl+Alt+H) / 병렬 스택. 이 줄에 동시에 서 있는 스레드는 몇 개인가요?
// Unreal Insights(-trace=default,task) 의 Timing 창에서는 작업 스레드 트랙에 ParallelFor 작업이 겹쳐 나타납니다.

// [JA] 入金はすべて済んだのに、ゴールドが足りません。
// [JA] Deposit にトレースポイント "{FPlatformTLS::GetCurrentThreadId()} 前 {Before} 入金 {Amount}" を設定してみてください。同じ Before を読んだスレッドはありますか？
// [JA] または Gold = Before + Amount の行にブレークポイント → デバッグ → ウィンドウ → スレッド（Ctrl+Alt+H）/ 並列スタック。この行に同時に止まっているスレッドはいくつですか？
// [JA] Unreal Insights（-trace=default,task）の Timing ウィンドウでは、ワーカースレッドのトラックに ParallelFor の処理が重なって表示されます。
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 두 스레드가 같은 값(예: 0)을 읽고 각자 더해서 쓰면, 나중에 쓴 쪽이 먼저 쓴 쪽을 덮습니다(lost update). 데이터 레이스입니다.
// RaceWindow 줄은 이 틈을 넓혀서 매번 일어나게 할 뿐입니다. 지워도 버그는 남고, "가끔" 일어나게 될 뿐입니다.
// 숫자 하나가 아니라 TArray::Add 였다면 같은 경쟁이 배열 크기와 메모리에서 일어나 크래시로 나타나기도 합니다.

// [JA] 2 つのスレッドが同じ値（例: 0）を読んでそれぞれ足して書くと、後から書いた方が先に書いた方を上書きします（lost update）。データ レースです。
// [JA] RaceWindow の行はこの隙間を広げて毎回起こるようにしているだけです。消してもバグは残り、「ときどき」起こるようになるだけです。
// [JA] 数値 1 つではなく TArray::Add だったら、同じ競合が配列のサイズとメモリで起こり、クラッシュとして現れることもあります。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 읽기-계산-쓰기 전체를 잠금으로 묶습니다. 구조체에 잠금을 하나 두고:
//     FCriticalSection Lock;
//     void Deposit(int32 Amount) { FScopeLock ScopeLock(&Lock); const int32 Before = Gold; BugHunt::RaceWindow(); Gold = Before + Amount; }
// 숫자 하나만 더하는 것이라면 원자 연산도 됩니다: std::atomic<int32> Gold; Gold.fetch_add(Amount);  (UE 의 TAtomic 은 std::atomic 으로 대체되는 중)
// 컨테이너는 원자 연산이 없으므로 잠금이 필요합니다. 애초에 각 작업이 결과를 자기 칸(Results[Index])에 쓰고
// 다 끝난 뒤 게임 스레드에서 합치면 잠금이 필요 없습니다.

// [JA] 読み取り・計算・書き込み全体をロックでまとめます。構造体にロックを 1 つ置いて:
// [JA]     FCriticalSection Lock;
// [JA]     void Deposit(int32 Amount) { FScopeLock ScopeLock(&Lock); const int32 Before = Gold; BugHunt::RaceWindow(); Gold = Before + Amount; }
// [JA] 数値 1 つを足すだけならアトミック演算でもできます: std::atomic<int32> Gold; Gold.fetch_add(Amount);  （UE の TAtomic は std::atomic に置き換えられつつあります）
// [JA] コンテナにはアトミック演算がないのでロックが必要です。そもそも各処理が結果を自分の枠（Results[Index]）に書き、
// [JA] すべて終わった後にゲームスレッドでまとめれば、ロックは要りません。
#pragma endregion
