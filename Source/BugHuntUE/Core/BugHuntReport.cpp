#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"

namespace BugHunt
{
namespace
{
int32 GPassed = 0;
int32 GFailed = 0;
}

void ResetTally()
{
	GPassed = 0;
	GFailed = 0;
}

int32 NumPassed() { return GPassed; }
int32 NumFailed() { return GFailed; }

void Print(const FString& Message, const FColor& Color)
{
	UE_LOG(LogBugHunt, Display, TEXT("%s"), *Message);
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 30.f, Color, Message);
	}
}

bool Expect(const FString& What, int64 Expected, int64 Actual)
{
	if (Expected == Actual)
	{
		++GPassed;
		Print(FString::Printf(TEXT("[PASS] %s (값: %lld)"), *What, Actual), FColor::Green);
		return true;
	}
	++GFailed;
	Print(FString::Printf(TEXT("[FAIL] %s (기대: %lld, 실제: %lld)"), *What, Expected, Actual), FColor::Red);
	return false;
}

bool ExpectTrue(const FString& What, bool bCondition)
{
	if (bCondition)
	{
		++GPassed;
		Print(FString::Printf(TEXT("[PASS] %s"), *What), FColor::Green);
		return true;
	}
	++GFailed;
	Print(FString::Printf(TEXT("[FAIL] %s"), *What), FColor::Red);
	return false;
}

void Skip(const FString& What, const FString& Reason)
{
	Print(FString::Printf(TEXT("[SKIP] %s: %s"), *What, *Reason), FColor::Yellow);
}

void Note(const FString& Message)
{
	Print(FString::Printf(TEXT("  %s"), *Message), FColor::White);
}

FString NetTag(const UObject* WorldContext)
{
	const UWorld* World = WorldContext ? WorldContext->GetWorld() : nullptr;
	if (!World)
	{
		return TEXT("[?]");
	}
	switch (World->GetNetMode())
	{
	case NM_Standalone:      return TEXT("[스탠드얼론]");
	case NM_DedicatedServer: return TEXT("[데디케이티드 서버]");
	case NM_ListenServer:    return TEXT("[리슨 서버]");
	case NM_Client:          return TEXT("[클라이언트]");
	default:                 return TEXT("[?]");
	}
}

void ExitIfAutomated()
{
	if (FParse::Param(FCommandLine::Get(), TEXT("BugHuntExit")))
	{
		UE_LOG(LogBugHunt, Display, TEXT("-BugHuntExit: 종료합니다."));
		RequestEngineExit(TEXT("BugHunt lab finished"));
	}
}
}
