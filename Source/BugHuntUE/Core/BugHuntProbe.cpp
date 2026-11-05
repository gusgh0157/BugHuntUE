#include "Core/BugHuntProbe.h"
#include "Misc/App.h"
#include "HAL/PlatformProcess.h"
#include <atomic>

namespace BugHunt
{
namespace
{
	bool GFixedFrameRate = false;
	std::atomic<int32> GRaceArrivals { 0 };
	std::atomic<int32> GRaceMisses { 0 };
	std::atomic<int32> GReallocCount { 0 };
}

void ResetProbes()
{
	SetFixedFrameRate(0.f);
	GRaceArrivals = 0;
	GRaceMisses = 0;
	GReallocCount = 0;
}

void SetFixedFrameRate(float Fps)
{
	if (Fps > 0.f)
	{
		FApp::SetFixedDeltaTime(1.0 / Fps);
		FApp::SetUseFixedTimeStep(true);
		GFixedFrameRate = true;
	}
	else if (GFixedFrameRate)
	{
		FApp::SetUseFixedTimeStep(false);
		GFixedFrameRate = false;
	}
}

void RaceWindow(int32 TimeoutMs)
{
	// 도착 순서가 홀수인 스레드는 짝(다음 도착)을 기다리고, 짝수인 스레드는 바로 지나간다.
	const int32 Arrival = ++GRaceArrivals;
	if (Arrival % 2 == 0)
	{
		return;
	}
	const double Deadline = FPlatformTime::Seconds() + TimeoutMs / 1000.0;
	while (GRaceArrivals.load() <= Arrival && FPlatformTime::Seconds() < Deadline)
	{
		FPlatformProcess::Sleep(0.0005f);
	}
	if (GRaceArrivals.load() <= Arrival)
	{
		++GRaceMisses;
	}
}

int32 RaceWindowMisses()
{
	return GRaceMisses.load();
}

void* FCountingMalloc::Realloc(void* Original, SIZE_T Count, uint32 Alignment)
{
	if (Count > 0)
	{
		++GReallocCount;
	}
	return FMemory::Realloc(Original, Count, Alignment);
}

void FCountingMalloc::Free(void* Original)
{
	FMemory::Free(Original);
}

int32 ArrayReallocCount()
{
	return GReallocCount.load();
}
}
