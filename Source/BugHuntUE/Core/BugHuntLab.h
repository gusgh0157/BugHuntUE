#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"
#include "BugHuntLab.generated.h"

// 모든 랩의 부모. 콘솔 명령 `BugHunt.Lab N`이 이 액터를 하나 스폰하고 StartLab()을 부른다.
// 랩 코드(=버그가 있는 게임 코드)는 각 LabNN_*.cpp에 있다. 이 파일에는 버그가 없다.
UCLASS(Abstract, NotPlaceable)
class BUGHUNTUE_API ABugHuntLab : public AActor
{
	GENERATED_BODY()

public:
	ABugHuntLab();

	void BeginLab(int32 InLabNumber, const FString& InTitle);

protected:
	virtual void StartLab() {}

	// 결과 요약을 찍는다. 랩이 만든 액터는 다음 랩을 실행할 때 정리된다 (그 전까지 World Outliner에서 볼 수 있다).
	void FinishLab();

	// Seconds초 뒤에 Func를 실행한다 (게임 시간 기준).
	void After(float Seconds, TFunction<void()> Func);

	// 멀티플레이 랩용. 명령줄에 -BugHuntWaitForClient 가 있으면 원격 플레이어가 접속할 때까지(최대 2분)
	// 기다렸다가 Func를 실행한다. 없으면 바로 실행한다. (별도 프로세스로 서버/클라이언트를 띄우는 자동 검증용)
	void WaitForClientThen(TFunction<void()> Func);

	static int32 CountRemotePlayers(const UWorld* World);

	// 이 랩 소유로 액터를 스폰한다.
	template <typename T>
	T* SpawnLabActor(const FVector& Location = FVector::ZeroVector)
	{
		FActorSpawnParameters Params;
		Params.Owner = this;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		return GetWorld()->SpawnActor<T>(T::StaticClass(), Location, FRotator::ZeroRotator, Params);
	}

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	int32 LabNumber = 0;
	FString LabTitle;

private:
	bool bFinished = false;
	int32 WaitTries = 0;
};
