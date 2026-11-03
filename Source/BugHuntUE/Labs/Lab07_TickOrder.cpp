#include "Labs/Lab07_TickOrder.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

ABH_BleedingHero::ABH_BleedingHero()
{
	PrimaryActorTick.bCanEverTick = true;
	// 이동/애니메이션이 다 끝난 뒤에 상태 이상을 처리한다.
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void ABH_BleedingHero::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Health > 0)
	{
		--Health;
	}
}

ABH_HealthBar::ABH_HealthBar()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABH_HealthBar::Attach(ABH_BleedingHero* InHero)
{
	Hero = InHero;
	DisplayedHealth = Hero->Health;
}

void ABH_HealthBar::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Hero)
	{
		DisplayedHealth = Hero->Health;
	}
}

// ---------------------------------------------------------------------------
// 아래는 검사 코드입니다. 버그는 위쪽에 있습니다.

ABH_Lab07::ABH_Lab07()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickGroup = TG_LastDemotable;
}

void ABH_Lab07::StartLab()
{
	Hero = SpawnLabActor<ABH_BleedingHero>();
	Bar = SpawnLabActor<ABH_HealthBar>();
	Bar->Attach(Hero);
	BugHunt::Note(FString::Printf(TEXT("영웅 HP %d, 매 프레임 1씩 출혈"), Hero->Health));
	SetActorTickEnabled(true);
}

void ABH_Lab07::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	++FramesChecked;
	if (Bar->DisplayedHealth != Hero->Health)
	{
		++FramesWrong;
		UE_LOG(LogBugHunt, Log, TEXT("  프레임 %llu: 실제 HP %d, HP 바 %d"), GFrameCounter, Hero->Health, Bar->DisplayedHealth);
	}
	if (Hero->Health == 0 && BarWhenHeroDied < 0)
	{
		BarWhenHeroDied = Bar->DisplayedHealth;
	}

	if (FramesChecked >= 40)
	{
		SetActorTickEnabled(false);
		BugHunt::Expect(TEXT("HP 바가 실제 HP와 다른 프레임 수"), 0, FramesWrong);
		BugHunt::Expect(TEXT("영웅이 쓰러진 프레임의 HP 바"), 0, BarWhenHeroDied);
		FinishLab();
	}
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// ABH_BleedingHero::Tick 과 ABH_HealthBar::Tick 에 추적점(중단점 우클릭 → 작업, "실행 계속" 체크)을 걸고
//     Hero {Health}      /      Bar reads {Hero->Health}
// 를 출력해 보세요. 출력 창(Ctrl+Alt+O)에서 둘 중 누가 먼저 찍히나요? 매 프레임 같은 순서인가요?
// 콘솔(~)에 dumpticks 를 입력하면 등록된 Tick 함수와 Tick 그룹 목록이 Output Log에 나옵니다.
#pragma endregion

#pragma region 힌트 2
// 언리얼은 한 프레임을 Tick 그룹 순서대로 돕니다:
//     TG_PrePhysics → TG_DuringPhysics → TG_PostPhysics → TG_PostUpdateWork → TG_LastDemotable
// 액터의 기본 그룹은 TG_PrePhysics 입니다. 영웅과 HP 바는 각각 어느 그룹인가요?
#pragma endregion

#pragma region 힌트 3
// "A 를 읽는 쪽은 A 를 바꾸는 쪽보다 뒤에 Tick 해야 한다."
// 같은 그룹 안에서의 순서는 보장되지 않습니다. 순서가 중요하면 AddTickPrerequisiteActor 로 명시하세요.
#pragma endregion
