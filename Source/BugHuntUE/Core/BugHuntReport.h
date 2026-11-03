#pragma once

#include "CoreMinimal.h"

// 랩 결과 출력. Output Log(카테고리 LogBugHunt)와 게임 화면 양쪽에 찍는다.
//   [PASS] 이름 (값: 100)
//   [FAIL] 이름 (기대: 100, 실제: 70)
namespace BugHunt
{
BUGHUNTUE_API void ResetTally();
BUGHUNTUE_API int32 NumPassed();
BUGHUNTUE_API int32 NumFailed();

BUGHUNTUE_API bool Expect(const FString& What, int64 Expected, int64 Actual);
BUGHUNTUE_API bool ExpectTrue(const FString& What, bool bCondition);
BUGHUNTUE_API void Skip(const FString& What, const FString& Reason);
BUGHUNTUE_API void Note(const FString& Message);
BUGHUNTUE_API void Print(const FString& Message, const FColor& Color);

// 멀티플레이 랩용: "[리슨 서버]", "[클라이언트]", "[스탠드얼론]" 같은 꼬리표
BUGHUNTUE_API FString NetTag(const UObject* WorldContext);

// 명령줄에 -BugHuntExit 가 있으면 (자동 검증용) 프로그램을 끝낸다.
BUGHUNTUE_API void ExitIfAutomated();
}
