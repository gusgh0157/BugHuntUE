#include "Labs/Lab23_Turret.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "TimerManager.h"

void ABH_Turret::Activate(ABH_TurretTarget* InTarget, float InInterval)
{
	Target = InTarget;
	GetWorldTimerManager().SetTimer(FireTimer, FTimerDelegate::CreateLambda([this]
	{
		Fire();
	}), InInterval, true);
}

void ABH_Turret::Fire()
{
	if (!Target)
	{
		return;
	}
	++ShotsFired;
	++Target->HitsTaken;
	Target->Health -= ShotDamage;
	UE_LOG(LogBugHunt, Log, TEXT("  %.2f초: %s 발사 %d -> 표적 HP %d"), GetWorld()->GetTimeSeconds(), *GetName(), ShotsFired, Target->Health);
}

void ABH_Lab23::StartLab()
{
	const float Interval = BugHunt::CaseFloat(0.2f, 0.12f, 0.25f);
	const int32 ShotsBeforeBreak = BugHunt::CaseInt(2, 1, 4);
	// 부서지는 시점: ShotsBeforeBreak 번째 발사와 그다음 발사의 한가운데
	const float BreakTime = Interval * (ShotsBeforeBreak + 0.5f);

	ABH_TurretTarget* Dummy = SpawnLabActor<ABH_TurretTarget>(FVector(500.f, 0.f, 100.f));
	ABH_Turret* Turret = SpawnLabActor<ABH_Turret>(FVector(0.f, 0.f, 100.f));
	Turret->Activate(Dummy, Interval);
	BugHunt::Note(FString::Printf(TEXT("포탑이 %.2f초마다 쏜다. %.2f초에 포탑을 부순다. 그 뒤로는 맞으면 안 된다."), Interval, BreakTime));

	After(BreakTime, [Turret]
	{
		BugHunt::Note(TEXT("포탑 파괴!"));
		Turret->Destroy();
	});

	After(BreakTime + 1.0f, [this, Dummy, ShotsBeforeBreak]
	{
		BugHunt::Expect(TEXT("표적이 맞은 횟수"), ShotsBeforeBreak, Dummy->HitsTaken);
		BugHunt::Expect(TEXT("표적 HP"), 100 - ShotsBeforeBreak * ABH_Turret::ShotDamage, Dummy->Health);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// Output Log 를 보면 "포탑 파괴!" 뒤에도 발사 줄이 찍힙니다. 누가 쏘고 있나요?
// ABH_Turret::Fire 첫 줄에 조건부 중단점을 거세요. 조건: ShotsFired >= 3 (숨은 입력이면 더 크게)
// 멈추면 호출 스택에서 누가 Fire 를 불렀는지 보고, 조사식에 this 를 넣어 펼쳐 보세요.
// 이 포탑은 지금 살아 있나요? (조사식: this->bActorIsBeingDestroyed)
#pragma endregion

#pragma region 힌트 2
// 호출 스택: FTimerManager::Tick → 람다 → Fire. 타이머가 아직 살아 있습니다.
// 액터가 파괴되면(EndPlay) 엔진은 그 액터에 "묶인" 타이머를 지웁니다 (ClearAllTimersForObject(this)).
// 그런데 CreateLambda 로 만든 델리게이트는 어느 객체에 묶여 있나요? 람다가 this 를 캡처한 것은
// 타이머 관리자 입장에서는 아무 의미가 없습니다. 그냥 주소 하나를 들고 있을 뿐입니다.
// 지금은 GC 전이라 this 의 메모리가 남아 있어서 "쏘기만" 합니다. GC 가 돈 뒤라면? (랩 4 를 떠올려 보세요)
#pragma endregion

#pragma region 힌트 3
// 델리게이트를 객체에 묶으세요. 어느 것이든 됩니다:
//     FTimerDelegate::CreateUObject(this, &ABH_Turret::Fire)
//     FTimerDelegate::CreateWeakLambda(this, [this] { Fire(); })
//     GetWorldTimerManager().SetTimer(FireTimer, this, &ABH_Turret::Fire, InInterval, true);
// 묶인 델리게이트는 객체가 사라지면 실행되지 않고, EndPlay 때 타이머도 같이 지워집니다.
// (EndPlay 를 오버라이드해서 ClearTimer(FireTimer) 해도 되지만, 바인딩 쪽을 고치는 것이 근본 해결입니다.)
#pragma endregion
