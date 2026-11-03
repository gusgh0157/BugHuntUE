#include "Core/BugHuntHang.h"
#include "Core/BugHuntReport.h"
#include "Core/BugHuntText.h"
#include "HAL/IConsoleManager.h"

namespace BugHunt
{
namespace
{
float GHangBudgetSeconds = 2.f;
FAutoConsoleVariableRef CVarHangBudget(
	TEXT("BugHunt.HangBudgetSeconds"),
	GHangBudgetSeconds,
	TEXT("10부 멈춤 랩의 루프/대기가 이 시간(초)을 넘으면 '응답 없음'으로 FAIL 처리하고 빠져나온다. 기본 2초. / 10部の停止ラボのループ／待機がこの時間（秒）を超えると「応答なし」として FAIL にし、抜け出します。既定 2 秒。"));
}

double HangBudgetSeconds()
{
	return FMath::Max(0.1f, GHangBudgetSeconds);
}

void ReportHang(const FString& What, const FString& Detail)
{
	ExpectTrue(Fmt<HangText::HangLine>(*Detail, *What), false);
}

FHangGuard::FHangGuard(const FString& InWhat, int64 InMaxIterations)
	: What(InWhat)
	, StartSeconds(FPlatformTime::Seconds())
	, BudgetSeconds(HangBudgetSeconds())
	, MaxIterations(InMaxIterations)
{
}

bool FHangGuard::Tripped()
{
	if (bTripped)
	{
		return true;
	}
	++Iterations;
	const double Elapsed = FPlatformTime::Seconds() - StartSeconds;
	if (Elapsed < BudgetSeconds && (MaxIterations <= 0 || Iterations <= MaxIterations))
	{
		return false;
	}
	bTripped = true;
	ReportHang(What, Fmt<HangText::Exceeded>(Elapsed * 1000.0, Iterations));
	return true;
}
}
