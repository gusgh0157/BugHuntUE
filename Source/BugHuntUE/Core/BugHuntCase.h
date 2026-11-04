#pragma once

#include "CoreMinimal.h"

// 랩 입력값. 평소(콘솔 명령, PIE)에는 README에 적힌 공개 입력을 쓰고,
// 채점기가 -BugHuntSeed=N 을 주면 그 시드로 만든 숨은 입력을 쓴다.
// "기대값을 코드에 박아 넣는" 수정은 숨은 입력에서 걸린다.
namespace BugHunt
{
// 0이면 공개 입력. BugHunt.Lab N 시드 로도 직접 줄 수 있다.
BUGHUNTUE_API int32 CaseSeed();
BUGHUNTUE_API void SetCaseSeed(int32 Seed);
inline bool IsHiddenCase() { return CaseSeed() != 0; }

// 랩을 시작할 때 부른다 (ABugHuntLab::BeginLab). 같은 시드면 같은 순서로 같은 값이 나온다.
BUGHUNTUE_API void ResetCase();

// 공개 입력이면 Public, 숨은 입력이면 [Min, Max] 범위의 값.
BUGHUNTUE_API int32 CaseInt(int32 Public, int32 Min, int32 Max);
BUGHUNTUE_API float CaseFloat(float Public, float Min, float Max);
}
