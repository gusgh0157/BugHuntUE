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

	// Seconds초 뒤에 Func를 실행한다 (게임 시간 기준). 0이면 다음 프레임.
	void After(float Seconds, TFunction<void()> Func);

	// 매 프레임 Done 을 확인하다가 참이 되면 Func 를 실행한다. 작업 스레드의 결과처럼 "언제 올지"가
	// PC 부하에 달린 것을 기다릴 때 쓴다 (고정 시간 After 는 느린 PC 에서 결과보다 먼저 판정해 버린다).
	// MaxRealSeconds(실제 시간) 안에 안 오면 그냥 Func 를 실행한다. 이것은 판정이 아니라 안전장치다.
	// [JA] 毎フレーム Done を確認し、真になったら Func を実行する。ワーカースレッドの結果のように「いつ来るか」が
	// [JA] PC の負荷次第のものを待つときに使う（固定時間の After は遅い PC で結果より先に判定してしまう）。
	// [JA] MaxRealSeconds（実時間）以内に来なければ、そのまま Func を実行する。これは判定ではなく安全装置。
	void WhenDone(TFunction<bool()> Done, float MaxRealSeconds, TFunction<void()> Func);

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
	void PollDone(TFunction<bool()> Done, double Deadline, TFunction<void()> Func);

	bool bFinished = false;
	int32 WaitTries = 0;
};
