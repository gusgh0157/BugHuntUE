#pragma once

#include "CoreMinimal.h"
#include "Commandlets/Commandlet.h"
#include "BugHuntGradeCommandlet.generated.h"

// 채점기. 모든 랩을 랩마다 새 프로세스(-game -nullrhi -BugHuntLab=N -BugHuntExit)로 돌려
// PASS / FAIL / TIMEOUT / CRASH / SKIP 를 매기고 표와 grade-report.json 을 남긴다.
// 보통은 프로젝트 폴더의 grade.bat 으로 실행한다.
//
//   UnrealEditor-Cmd.exe BugHuntUE.uproject -run=BugHuntGrade [옵션]
//     -labs=1-4,22      채점할 랩 (기본: 전부)
//     -seed=N           숨은 입력 시드 (기본: 실행할 때마다 무작위)
//     -public           숨은 입력 대신 README 의 공개 입력으로 채점
//     -timeout=초       랩 하나(프로세스 시작 포함)의 제한 시간 (기본 180)
//     -gpu              -nullrhi 없이 실행하고 GPU 가 필요한 랩도 채점
//     -out=경로         JSON 결과 파일 (기본: 프로젝트 폴더의 grade-report.json)
UCLASS()
class UBugHuntGradeCommandlet : public UCommandlet
{
	GENERATED_BODY()

public:
	UBugHuntGradeCommandlet();

	virtual int32 Main(const FString& Params) override;
};
