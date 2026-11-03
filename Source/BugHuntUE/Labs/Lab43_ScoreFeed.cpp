#include "Labs/Lab43_ScoreFeed.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Async/Async.h"

void ABH_ScoreFeed::AddScore(const FString& Player, int32 Points)
{
	// 엔진의 액터/컴포넌트 함수 대부분에 있는 것과 같은 검사. (이 줄은 버그가 아닙니다)
	check(IsInGameThread());
	Scores.FindOrAdd(Player) += Points;
	++Updates;
	UE_LOG(LogBugHunt, Log, TEXT("  점수판: %s +%d (합계 %d)"), *Player, Points, Scores[Player]);
}

void ABH_Lab43::StartLab()
{
	ABH_ScoreFeed* Feed = SpawnLabActor<ABH_ScoreFeed>();
	// [JA] エンジンのアクター／コンポーネント関数の大半にあるのと同じチェック。（この行はバグではありません）
	const int32 Points = BugHunt::CaseInt(250, 50, 900);
	BugHunt::Note(FString::Printf(TEXT("리플레이를 분석해서 하이라이트 점수 %d 점을 점수판에 올린다. 분석은 무거우니 작업 스레드에서."), Points));

	TWeakObjectPtr<ABH_ScoreFeed> WeakFeed = Feed;
	Async(EAsyncExecution::Thread, [WeakFeed, Points]   // 분석 전용 스레드를 하나 띄운다
	{
		// (리플레이 분석: 무거운 계산이라고 치자)
		FPlatformProcess::Sleep(0.05f);
		const FString Mvp = TEXT("Aria");

		if (ABH_ScoreFeed* Target = WeakFeed.Get())
		{
			Target->AddScore(Mvp, Points);
		}
	// [JA] 解析専用のスレッドを 1 つ立てる
	});

		// [JA] （リプレイ解析: 重い計算だとしよう）
	After(1.0f, [this, Feed, Points]
	{
		BugHunt::Expect(TEXT("점수판 갱신 횟수"), 1, Feed->Updates);
		BugHunt::Expect(TEXT("Aria 점수"), Points, Feed->Scores.FindRef(TEXT("Aria")));
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// Assertion failed: IsInGameThread(). 1부에서 한 대로 호출 스택을 거슬러 올라가 보세요. 맨 아래(스택의 시작)는 어디인가요?
// 디버그 → 창 → 스레드 에서 멈춘 스레드의 이름은 무엇인가요? GameThread 인가요?
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// UObject, 액터, 컴포넌트, 월드는 게임 스레드에서만 읽고 쓰는 것이 원칙입니다. 엔진은 잠금 없이 이 객체들을 다루기 때문에,
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// 다른 스레드에서 만지면 게임 스레드와 동시에 같은 TMap 을 고치는 데이터 레이스가 됩니다. check(IsInGameThread()) 는 그걸 그 자리에서 잡는 장치입니다.
// TWeakObjectPtr::Get() 도 다른 스레드에서 부르면 그 순간 GC 와 경쟁합니다.
#pragma endregion

#pragma region 힌트 3 / ヒント 3

// [JA] Assertion failed: IsInGameThread()。第 1 部でやったように呼び出し履歴をさかのぼってみてください。いちばん下（スタックの始まり）はどこですか？
// [JA] デバッグ → ウィンドウ → スレッド で、止まったスレッドの名前は何ですか？ GameThread ですか？
// 무거운 계산만 작업 스레드에서 하고, 결과는 게임 스레드로 보내서 적용합니다(랩 44 의 데드락 수정과 같은 방법):
//     Async(EAsyncExecution::Thread, [WeakFeed, Points]
//     {
//         const FString Mvp = ...;                                   // 계산
//         AsyncTask(ENamedThreads::GameThread, [WeakFeed, Mvp, Points]
//         {

// [JA] UObject、アクター、コンポーネント、ワールドはゲームスレッドでのみ読み書きするのが原則です。エンジンはロックなしでこれらのオブジェクトを扱うため、
// [JA] 別のスレッドから触ると、ゲームスレッドと同時に同じ TMap を書き換えるデータ レースになります。check(IsInGameThread()) はそれをその場で捕まえる仕組みです。
// [JA] TWeakObjectPtr::Get() も別のスレッドから呼ぶと、その瞬間に GC と競合します。
//             if (ABH_ScoreFeed* Target = WeakFeed.Get()) { Target->AddScore(Mvp, Points); }   // 적용은 게임 스레드에서
//         });
//     });
// 작업 스레드 람다에는 UObject 포인터 대신 계산에 필요한 값만 복사해서 넘기는 습관을 들이세요.
#pragma endregion

// [JA] 重い計算だけをワーカースレッドで行い、結果はゲームスレッドに送って適用します（ラボ 44 のデッドロック修正と同じ方法）:
// [JA]     Async(EAsyncExecution::Thread, [WeakFeed, Points]
// [JA]     {
// [JA]         const FString Mvp = ...;                                   // 計算
// [JA]         AsyncTask(ENamedThreads::GameThread, [WeakFeed, Mvp, Points]
// [JA]         {
// [JA]             if (ABH_ScoreFeed* Target = WeakFeed.Get()) { Target->AddScore(Mvp, Points); }   // 適用はゲームスレッドで
// [JA]         });
// [JA]     });
// [JA] ワーカースレッドのラムダには、UObject ポインタではなく計算に必要な値だけをコピーして渡す習慣をつけてください。
