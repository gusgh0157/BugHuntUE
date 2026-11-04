#include "Labs/Lab09_Archer.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "TimerManager.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab09Text
{
BH_TEXT(Killed,   "  화살 %d: %s 처치! 점수 %d", "  矢 %d: %s を撃破！ スコア %d");
BH_TEXT(Setup,    "허수아비 3개 (HP 30). 화살 10 피해 × 9발이면 정확히 다 쓰러진다.", "かかし 3 体（HP 30）。矢 10 ダメージ × 9 発でちょうど全部倒れる。");
BH_TEXT(Shot,     "쏜 화살", "撃った矢");
BH_TEXT(Standing, "서 있는 허수아비", "立っているかかし");
BH_TEXT(Score,    "점수", "スコア");
}

namespace
{
constexpr int32 ArrowDamage = 10;
constexpr int32 KillReward = 100;
constexpr int32 MaxArrows = 9;
}

bool ABH_Dummy::ReceiveArrow(int32 Damage)
{
	Health -= Damage;
	if (Health <= 0)
	{
		Destroy();
		return true;
	}
	return false;
}

void ABH_Archer::StartShooting()
{
	GetWorldTimerManager().SetTimer(VolleyTimer, this, &ABH_Archer::Volley, 0.1f, true);
}

void ABH_Archer::StopShooting()
{
	GetWorldTimerManager().ClearTimer(VolleyTimer);
}

ABH_Dummy* ABH_Archer::FindNearestTarget() const
{
	ABH_Dummy* Best = nullptr;
	float BestDistance = TNumericLimits<float>::Max();
	for (ABH_Dummy* Dummy : Candidates)
	{
		if (!IsValid(Dummy))
		{
			continue;
		}
		const float Distance = GetDistanceTo(Dummy);
		if (Distance < BestDistance)
		{
			Best = Dummy;
			BestDistance = Distance;
		}
	}
	return Best;
}

void ABH_Archer::Volley()
{
	if (ArrowsShot >= MaxArrows)
	{
		StopShooting();
		return;
	}

	if (!CurrentTarget)
	{
		CurrentTarget = FindNearestTarget();
	}
	if (!CurrentTarget)
	{
		return;
	}

	++ArrowsShot;
	if (CurrentTarget->ReceiveArrow(ArrowDamage))
	{
		Score += KillReward;
		UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab09Text::Killed>(ArrowsShot, *CurrentTarget->GetName(), Score));
	}
}

void ABH_Lab09::StartLab()
{
	ABH_Archer* Archer = SpawnLabActor<ABH_Archer>();
	for (int32 i = 0; i < 3; ++i)
	{
		Archer->Candidates.Add(SpawnLabActor<ABH_Dummy>(FVector(300.f + 200.f * i, 0.f, 0.f)));
	}
	BugHunt::Note(BugHunt::Tr<Lab09Text::Setup>());
	Archer->StartShooting();

	After(1.5f, [this, Archer]
	{
		int32 Standing = 0;
		for (ABH_Dummy* Dummy : Archer->Candidates)
		{
			if (IsValid(Dummy))
			{
				++Standing;
			}
		}
		BugHunt::Expect(BugHunt::Tr<Lab09Text::Shot>(), MaxArrows, Archer->ArrowsShot);
		BugHunt::Expect(BugHunt::Tr<Lab09Text::Standing>(), 0, Standing);
		BugHunt::Expect(BugHunt::Tr<Lab09Text::Score>(), 3 * KillReward, Archer->Score);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// Output Log를 보세요. "처치!" 가 몇 번 찍히고, 처치한 허수아비 이름은 매번 다른가요?
// Volley 의 Score += 줄에 중단점을 걸고, 두 번째로 멈췄을 때 지역 창에서 CurrentTarget 을 펼쳐
// Health 와 이름을 보세요. 이미 쓰러진 허수아비 아닌가요?

// [JA] アウトプットログを見てください。「撃破！」は何回出ていて、撃破したかかしの名前は毎回違いますか？
// [JA] Volley の Score += の行にブレークポイントを設定し、2 回目に止まったときにローカル ウィンドウで CurrentTarget を展開して
// [JA] Health と名前を見てください。すでに倒れたかかしではありませんか？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 쓰러진 허수아비는 Destroy() 됐는데 CurrentTarget 은 nullptr 가 아닙니다.
// Destroy() 는 액터를 "곧 지울 것(Garbage)"으로 표시할 뿐, 그 메모리와 그 액터를 가리키는 포인터는
// 다음 GC 때까지 그대로 남습니다. 그래서 if (!CurrentTarget) 은 통과하지 않습니다.
// FindNearestTarget 은 같은 문제를 어떻게 피하고 있나요?

// [JA] 倒れたかかしは Destroy() されたのに、CurrentTarget は nullptr ではありません。
// [JA] Destroy() はアクターを「間もなく消すもの（Garbage）」として印を付けるだけで、そのメモリとそのアクターを指すポインタは
// [JA] 次の GC までそのまま残ります。そのため if (!CurrentTarget) は通りません。
// [JA] FindNearestTarget は同じ問題をどうやって避けていますか？
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// UObject 포인터가 "쓸 수 있는가"는 nullptr 비교가 아니라 IsValid(Ptr) 로 확인합니다.
// (nullptr 도 아니고 Garbage 표시도 없어야 true)

// [JA] UObject ポインタが「使えるか」は nullptr との比較ではなく IsValid(Ptr) で確認します。
// [JA] （nullptr でもなく、Garbage の印もないときに true）
#pragma endregion
