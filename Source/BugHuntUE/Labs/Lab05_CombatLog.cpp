#include "Labs/Lab05_CombatLog.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

void ABH_Fighter::TakeHit(int32 Damage)
{
	Health -= Damage;
	OnDamaged.Broadcast(this, Damage);
}

void UBH_CombatLogPanel::Open(ABH_Fighter* InTarget)
{
	Target = InTarget;
	Lines.Reset();
	Target->OnDamaged.AddUObject(this, &UBH_CombatLogPanel::HandleDamaged);
}

void UBH_CombatLogPanel::Close()
{
	Lines.Reset();
}

void UBH_CombatLogPanel::HandleDamaged(AActor* Victim, int32 Damage)
{
	Lines.Add(FString::Printf(TEXT("%s: -%d"), *Victim->GetName(), Damage));
	++NotificationsShown;
}

void ABH_Lab05::StartLab()
{
	ABH_Fighter* Knight = SpawnLabActor<ABH_Fighter>();
	Panel = NewObject<UBH_CombatLogPanel>(this);

	Panel->Open(Knight);
	Knight->TakeHit(10);
	Panel->Close();

	Knight->TakeHit(5);     // 창이 닫혀 있을 때 맞은 피해는 기록하지 않는다.

	Panel->Open(Knight);
	Knight->TakeHit(7);

	for (const FString& Line : Panel->Lines)
	{
		BugHunt::Note(FString::Printf(TEXT("전투 기록: %s"), *Line));
	}
	BugHunt::Expect(TEXT("다시 연 창의 기록 줄 수"), 1, Panel->Lines.Num());
	BugHunt::Expect(TEXT("띄운 알림 총합 (10, 7)"), 2, Panel->NotificationsShown);
	BugHunt::Expect(TEXT("기사 HP"), 78, Knight->Health);
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 크래시가 없습니다. HandleDamaged 에 중단점을 걸고 F5 를 누를 때마다 지역 창의 Damage 를 적으세요.
// 기대: 10, 7 (두 번). 실제로는 몇 번, 어떤 값으로 멈추나요?
// 멈출 때마다 호출 스택 두 번째 줄 아래의 ABH_Fighter::TakeHit 프레임을 보면 어느 TakeHit 호출인지 알 수 있습니다.
#pragma endregion

#pragma region 힌트 2
// 중단점을 우클릭 → 작업(Actions)으로 바꿔서 "HandleDamaged {Damage} 받음" 을 출력하고 "실행 계속"을 체크하면
// 멈추지 않고 출력 창에 순서가 남습니다. (추적점)
// ABH_Fighter::TakeHit 의 Broadcast 줄에서 조사식에 OnDamaged 를 넣고 펼쳐 보세요.
// InvocationList 안에 등록된 함수가 몇 개 있나요? 처음, Close 뒤, 다시 Open 뒤를 비교하세요.
#pragma endregion

#pragma region 힌트 3
// AddUObject 는 객체가 살아 있는 한 계속 불립니다. 창을 닫는다고 자동으로 빠지지 않습니다.
// Open 에서 Add 했다면 Close 에서는? (RemoveAll(this) 또는 Add 가 돌려주는 FDelegateHandle 로 Remove)
#pragma endregion
