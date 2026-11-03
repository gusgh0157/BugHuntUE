#include "Labs/Lab11_RepNotify.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Net/UnrealNetwork.h"

ABH_NetHero::ABH_NetHero()
{
	bReplicates = true;
	bAlwaysRelevant = true;
	SetNetUpdateFrequency(30.f);
}

void ABH_NetHero::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABH_NetHero, Health);
}

void ABH_NetHero::ApplyDamage(int32 Damage)
{
	check(HasAuthority());
	Health = FMath::Max(0, Health - Damage);
	UE_LOG(LogBugHunt, Log, TEXT("  %s 피해 -%d -> HP %d"), *BugHunt::NetTag(this), Damage, Health);
}

void ABH_NetHero::OnRep_Health()
{
	UpdateHud();
}

void ABH_NetHero::UpdateHud()
{
	HudHealth = Health;
	UE_LOG(LogBugHunt, Log, TEXT("  %s HP 바 갱신 -> %d"), *BugHunt::NetTag(this), HudHealth);
}

void ABH_NetHero::Multicast_CheckHud_Implementation(int32 ExpectedHealth)
{
	const FString Tag = BugHunt::NetTag(this);
	BugHunt::Expect(FString::Printf(TEXT("%s 복제된 HP"), *Tag), ExpectedHealth, Health);
	BugHunt::Expect(FString::Printf(TEXT("%s 화면의 HP 바"), *Tag), ExpectedHealth, HudHealth);

	if (GetNetMode() == NM_Client)
	{
		BugHunt::ExitIfAutomated();
	}
}

// ---------------------------------------------------------------------------
// 아래는 서버에서 도는 시나리오입니다.

void ABH_Lab11::StartLab()
{
	if (GetNetMode() == NM_Standalone)
	{
		BugHunt::Note(TEXT("스탠드얼론으로 실행 중. 리슨 서버 + 클라이언트로도 실행해 보세요 (README 참고)."));
	}

	const int32 Damage = BugHunt::CaseInt(15, 5, 30);
	WaitForClientThen([this, Damage]
	{
		ABH_NetHero* Hero = SpawnLabActor<ABH_NetHero>(FVector(0.f, 0.f, 100.f));
		BugHunt::Note(FString::Printf(TEXT("영웅 HP 100. 0.5초 간격으로 %d씩 세 번 맞는다."), Damage));

		After(0.5f, [Hero, Damage] { Hero->ApplyDamage(Damage); });
		After(1.0f, [Hero, Damage] { Hero->ApplyDamage(Damage); });
		After(1.5f, [Hero, Damage] { Hero->ApplyDamage(Damage); });

		// 복제될 시간을 충분히 준 뒤 모든 머신에서 각자 검사한다.
		After(2.5f, [Hero, Damage] { Hero->Multicast_CheckHud(100 - 3 * Damage); });
		After(3.5f, [this] { FinishLab(); });
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 이 랩은 PIE 설정을 바꿔서 두 번 실행해 보세요.
//   ① 플레이 버튼 옆 ⋮ → Number of Players 2, Net Mode "Play As Listen Server"
//   ② 서버 창(제목에 Server)에서 콘솔을 열고 BugHunt.Lab 11
// Output Log에서 [리슨 서버] 줄과 [클라이언트] 줄을 비교하세요. 어느 쪽만 [FAIL] 인가요?
// "HP 바 갱신" 로그는 어느 머신에서 찍히나요?
#pragma endregion

#pragma region 힌트 2
// OnRep_Health 와 ApplyDamage 에 중단점을 거세요. PIE의 서버와 클라이언트는 같은 프로세스라
// 한 디버거에서 둘 다 멈춥니다. 멈출 때마다 조사식에 GetNetMode() 를 넣어 누구인지 보세요
// (NM_ListenServer / NM_Client). 서버에서 OnRep_Health 가 한 번이라도 불리나요?
#pragma endregion

#pragma region 힌트 3
// C++ 의 RepNotify(ReplicatedUsing)는 "복제로 값을 받은 쪽"에서만 불립니다. 값을 직접 바꾼 서버에서는 안 불립니다.
// (블루프린트의 RepNotify 는 서버에서도 불려서 더 헷갈립니다.)
// 리슨 서버 호스트도 화면이 있는 플레이어입니다. 서버에서 값을 바꾼 곳에서 무엇을 해 줘야 할까요?
#pragma endregion
