#include "Core/BugHuntCase.h"
#include "Math/RandomStream.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace BugHunt
{
namespace
{
int32 GSeed = INDEX_NONE;   // INDEX_NONE: 아직 명령줄을 읽지 않음
FRandomStream GStream;
}

int32 CaseSeed()
{
	if (GSeed == INDEX_NONE)
	{
		GSeed = 0;
		FParse::Value(FCommandLine::Get(), TEXT("BugHuntSeed="), GSeed);
	}
	return GSeed;
}

void SetCaseSeed(int32 Seed)
{
	GSeed = Seed;
}

void ResetCase()
{
	GStream.Initialize(CaseSeed());
}

int32 CaseInt(int32 Public, int32 Min, int32 Max)
{
	return IsHiddenCase() ? GStream.RandRange(Min, Max) : Public;
}

float CaseFloat(float Public, float Min, float Max)
{
	return IsHiddenCase() ? GStream.FRandRange(Min, Max) : Public;
}
}
