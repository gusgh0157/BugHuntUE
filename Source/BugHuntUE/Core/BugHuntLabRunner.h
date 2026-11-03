#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "BugHuntLabRunner.generated.h"

namespace BugHunt
{
// 랩 Number를 World에서 실행한다. 콘솔 명령 BugHunt.Lab N 과 같다.
BUGHUNTUE_API void RunLab(UWorld* World, int32 Number);

struct FLabInfo
{
	int32 Number = 0;
	FString Title;
	FString Symptom;
	int32 Part = 0;
	bool bNetworked = false;   // 서버 + 클라이언트 두 프로세스로 검사한다 (-BugHuntWaitForClient)
	bool bNeedsGPU = false;    // -nullrhi 로는 판정할 수 없다
};

// 채점기용 랩 목록
BUGHUNTUE_API TArray<FLabInfo> GetLabs();
}

// -BugHuntLab=N 으로 실행하면 게임 월드가 시작될 때 랩을 자동으로 실행한다.
UCLASS()
class UBugHuntAutoRunSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
};
