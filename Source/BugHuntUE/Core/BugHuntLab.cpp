#include "Core/BugHuntLab.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "TimerManager.h"

ABugHuntLab::ABugHuntLab()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

void ABugHuntLab::BeginLab(int32 InLabNumber, const FString& InTitle)
{
	LabNumber = InLabNumber;
	LabTitle = InTitle;
	BugHunt::ResetTally();
	BugHunt::Print(FString::Printf(TEXT("==== 랩 %d: %s ===="), LabNumber, *LabTitle), FColor::Cyan);
	StartLab();
}

void ABugHuntLab::FinishLab()
{
	if (bFinished)
	{
		return;
	}
	bFinished = true;

	const int32 Failed = BugHunt::NumFailed();
	const FColor Color = Failed == 0 ? FColor::Green : FColor::Red;
	BugHunt::Print(FString::Printf(TEXT("==== 랩 %d 결과: PASS %d / FAIL %d ===="),
		LabNumber, BugHunt::NumPassed(), Failed), Color);
	BugHunt::ExitIfAutomated();
}

void ABugHuntLab::After(float Seconds, TFunction<void()> Func)
{
	// SetTimer 는 Rate <= 0 이면 타이머를 지우기만 하므로, 0초는 "다음 프레임"으로 처리한다.
	if (Seconds <= 0.f)
	{
		GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, MoveTemp(Func)));
		return;
	}
	FTimerHandle Unused;
	GetWorldTimerManager().SetTimer(Unused, FTimerDelegate::CreateWeakLambda(this, MoveTemp(Func)), Seconds, false);
}

int32 ABugHuntLab::CountRemotePlayers(const UWorld* World)
{
	int32 Remote = 0;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (It->IsValid() && !(*It)->IsLocalController())
		{
			++Remote;
		}
	}
	return Remote;
}

void ABugHuntLab::WaitForClientThen(TFunction<void()> Func)
{
	const bool bWait = FParse::Param(FCommandLine::Get(), TEXT("BugHuntWaitForClient"));
	if (!bWait || GetNetMode() == NM_Standalone || CountRemotePlayers(GetWorld()) > 0 || ++WaitTries > 240)
	{
		Func();
		return;
	}
	After(0.5f, [this, Func = MoveTemp(Func)]() mutable { WaitForClientThen(MoveTemp(Func)); });
}

void ABugHuntLab::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		for (TActorIterator<AActor> It(World); It; ++It)
		{
			if (*It != this && It->IsOwnedBy(this))
			{
				It->Destroy();
			}
		}
	}
	Super::EndPlay(EndPlayReason);
}
