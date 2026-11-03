#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntText.h"

namespace ReportText
{
// [PASS]/[FAIL]/[SKIP] 표시는 채점기가 읽으므로 두 언어 모두 그대로 둔다. / [PASS]/[FAIL]/[SKIP] は採点機が読むので両言語ともそのまま。
BH_TEXT(PassValue,   "[PASS] %s (값: %lld)",            "[PASS] %s (値: %lld)");
BH_TEXT(FailValue,   "[FAIL] %s (기대: %lld, 실제: %lld)", "[FAIL] %s (期待: %lld, 実際: %lld)");
BH_TEXT(Standalone,  "[스탠드얼론]",       "[スタンドアロン]");
BH_TEXT(Dedicated,   "[데디케이티드 서버]", "[専用サーバー]");
BH_TEXT(ListenServer,"[리슨 서버]",         "[リッスンサーバー]");
BH_TEXT(Client,      "[클라이언트]",       "[クライアント]");
BH_TEXT(Exiting,     "-BugHuntExit: 종료합니다.", "-BugHuntExit: 終了します。");
}

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
