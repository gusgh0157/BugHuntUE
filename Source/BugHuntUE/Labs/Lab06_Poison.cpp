#include "Labs/Lab06_Poison.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "TimerManager.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab06Text
{
BH_TEXT(Tick,     "  %.2f초: 독 피해 -%d -> HP %d", "  %.2f 秒: 毒ダメージ -%d -> HP %d");
BH_TEXT(Plan,     "0초 독 → 1.0초 독 다시 걸림 → 2.1초 정화 → 3.2초 결과 확인 (3초쯤 걸립니다)", "0 秒 毒 → 1.0 秒 再び毒 → 2.1 秒 浄化 → 3.2 秒 結果確認（3 秒ほどかかります）");
BH_TEXT(AfterCln, "정화 후 들어온 독 피해 횟수", "浄化後に入った毒ダメージの回数");
BH_TEXT(FinalHp,  "최종 HP (독 피해 4번)", "最終 HP（毒ダメージ 4 回）");
}

namespace
{
constexpr float PoisonInterval = 0.4f;
constexpr int32 PoisonDamage = 5;
}

void ABH_PoisonVictim::ApplyPoison()
{
	bPoisoned = true;
	GetWorldTimerManager().SetTimer(PoisonTimer, this, &ABH_PoisonVictim::PoisonTick, PoisonInterval, true);
}

void ABH_PoisonVictim::RefreshPoison()
{
	// 독에 다시 걸렸다: 주기를 처음부터 다시 센다.
	// [JA] もう一度毒にかかった: 周期を最初から数え直す。
	FTimerHandle NewTimer;
	GetWorldTimerManager().SetTimer(NewTimer, this, &ABH_PoisonVictim::PoisonTick, PoisonInterval, true);
}

void ABH_PoisonVictim::Cleanse()
{
	bPoisoned = false;
	GetWorldTimerManager().ClearTimer(PoisonTimer);
}

void ABH_PoisonVictim::PoisonTick()
{
	Health -= PoisonDamage;
	if (!bPoisoned)
	{
		++TicksAfterCleanse;
	}
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab06Text::Tick>(GetWorld()->GetTimeSeconds(), PoisonDamage, Health));
}

void ABH_Lab06::StartLab()
{
	ABH_PoisonVictim* Victim = SpawnLabActor<ABH_PoisonVictim>();
	BugHunt::Note(BugHunt::Tr<Lab06Text::Plan>());

	Victim->ApplyPoison();                           // 0.4, 0.8초에 피해
	// [JA] 0.4、0.8 秒にダメージ
	After(1.0f, [Victim] { Victim->RefreshPoison(); });  // 1.4, 1.8초에 피해
	// [JA] 1.4、1.8 秒にダメージ
	After(2.1f, [Victim] { Victim->Cleanse(); });        // 이후 피해 없음
	// [JA] 以降ダメージなし

	After(3.2f, [this, Victim]
	{
		BugHunt::Expect(BugHunt::Tr<Lab06Text::AfterCln>(), 0, Victim->TicksAfterCleanse);
		BugHunt::Expect(BugHunt::Tr<Lab06Text::FinalHp>(), 100 - 4 * PoisonDamage, Victim->Health);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// Output Log에서 LogBugHunt 줄의 시각을 보세요. 1.0초가 지난 뒤 피해 간격이 0.4초로 일정한가요?
// 정화(2.1초) 뒤에도 피해가 들어오나요?
// 랩이 도는 3초 동안 콘솔(~ 키)에 ListTimers 를 입력하면 지금 살아 있는 타이머 목록이 Output Log에 찍힙니다.
// PoisonTick 을 부르는 타이머가 몇 개인가요?

// [JA] アウトプットログで LogBugHunt の行の時刻を見てください。1.0 秒を過ぎた後、ダメージの間隔は 0.4 秒で一定ですか？
// [JA] 浄化（2.1 秒）の後もダメージが入っていますか？
// [JA] ラボが動いている 3 秒の間にコンソール（~ キー）で ListTimers を入力すると、今生きているタイマーの一覧がアウトプットログに出力されます。
// [JA] PoisonTick を呼ぶタイマーはいくつありますか？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// Cleanse 에 중단점을 걸고, ClearTimer(PoisonTimer) 를 넘기기 전 조사식에 PoisonTimer 를 넣어 보세요.
// 이 핸들은 어느 SetTimer 가 채운 핸들인가요? RefreshPoison 이 만든 타이머의 핸들은 지금 어디에 있나요?

// [JA] Cleanse にブレークポイントを設定し、ClearTimer(PoisonTimer) を越える前にウォッチに PoisonTimer を入れてみてください。
// [JA] このハンドルはどの SetTimer が埋めたハンドルですか？ RefreshPoison が作ったタイマーのハンドルは今どこにありますか？
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// FTimerHandle 은 "타이머를 다시 찾을 수 있는 유일한 열쇠"입니다. 지역 변수에 받으면 함수가 끝나는 순간 열쇠를 잃고,
// 반복(loop) 타이머는 액터가 사라질 때까지 영원히 돕니다.
// 같은 멤버 핸들로 SetTimer 를 다시 부르면 기존 타이머는 자동으로 지워지고 새로 시작합니다.

// [JA] FTimerHandle は「タイマーをもう一度見つけられる唯一の鍵」です。ローカル変数で受け取ると関数が終わった瞬間に鍵を失い、
// [JA] 繰り返し（loop）タイマーはアクターが消えるまで永遠に回り続けます。
// [JA] 同じメンバーのハンドルで SetTimer をもう一度呼ぶと、既存のタイマーは自動的に消されて新しく始まります。
#pragma endregion
