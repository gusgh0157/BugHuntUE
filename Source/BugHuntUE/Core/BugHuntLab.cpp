#include "Core/BugHuntLab.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "Core/BugHuntText.h"
#include "BugHuntUE.h"
#include "EngineUtils.h"
#include "GameFramework/PlayerController.h"
#include "Misc/CommandLine.h"
#include "TimerManager.h"

namespace LabText
{
BH_TEXT(Header, "==== 랩 %d: %s ====", "==== ラボ %d: %s ====");
BH_TEXT(Hidden, "숨은 입력 (시드 %d). 지원하는 랩은 공개 입력과 다른 값으로 돈다.", "隠し入力（シード %d）。対応するラボは公開入力とは別の値で動きます。");
// 채점기가 읽는 줄. "결과:" 뒤의 "PASS n / FAIL n" 은 두 언어 모두 같다. / 採点機が読む行。「結果:」の後ろの "PASS n / FAIL n" は両言語で共通。
BH_TEXT(Result, "==== 랩 %d 결과: PASS %d / FAIL %d ====", "==== ラボ %d 結果: PASS %d / FAIL %d ====");
}

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
	BugHunt::ResetCase();
	BugHunt::ResetProbes();
	BugHunt::Print(BugHunt::Fmt<LabText::Header>(LabNumber, *LabTitle), FColor::Cyan);
	if (BugHunt::IsHiddenCase())
	{
		BugHunt::Note(BugHunt::Fmt<LabText::Hidden>(BugHunt::CaseSeed()));
	}
	StartLab();
}

void ABugHuntLab::FinishLab()
{
	if (bFinished)
	{
		return;
	}
	bFinished = true;
	BugHunt::SetFixedFrameRate(0.f);

	const int32 Failed = BugHunt::NumFailed();
	const FColor Color = Failed == 0 ? FColor::Green : FColor::Red;
	BugHunt::Print(BugHunt::Fmt<LabText::Result>(
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

void ABugHuntLab::WhenDone(TFunction<bool()> Done, float MaxRealSeconds, TFunction<void()> Func)
{
	PollDone(MoveTemp(Done), FPlatformTime::Seconds() + MaxRealSeconds, MoveTemp(Func));
}

void ABugHuntLab::PollDone(TFunction<bool()> Done, double Deadline, TFunction<void()> Func)
{
	if (Done() || FPlatformTime::Seconds() > Deadline)
	{
		Func();
		return;
	}
	GetWorldTimerManager().SetTimerForNextTick(FTimerDelegate::CreateWeakLambda(this, [this, Done, Deadline, Func]
	{
		PollDone(Done, Deadline, Func);
	}));
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
