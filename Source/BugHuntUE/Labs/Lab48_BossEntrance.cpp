#include "Labs/Lab48_BossEntrance.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Misc/CoreDelegates.h"

ABH_BossArena::ABH_BossArena()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	// (임시 외형으로 엔진 기본 도형을 쓴다. 다른 랩이 쓰지 않는 것이어야 처음에 메모리에 없다.)
	BossMesh = TSoftObjectPtr<UStaticMesh>(FSoftObjectPath(TEXT("/Engine/BasicShapes/Cone.Cone")));
}

void ABH_BossArena::BeginPlay()
{
	Super::BeginPlay();
}

AStaticMeshActor* ABH_BossArena::SpawnBoss()
{
	UStaticMesh* Mesh = BossMesh.LoadSynchronous();

	FActorSpawnParameters Params;
	Params.Owner = GetOwner();
	AStaticMeshActor* Boss = GetWorld()->SpawnActor<AStaticMeshActor>(GetActorLocation(), FRotator::ZeroRotator, Params);
	Boss->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
	Boss->GetStaticMeshComponent()->SetStaticMesh(Mesh);
	Boss->SetActorScale3D(FVector(4.f));
	return Boss;
}

void ABH_Lab48::StartLab()
{
	ABH_BossArena* Arena = SpawnLabActor<ABH_BossArena>(FVector(1000.f, 0.f, 0.f));
	BugHunt::Note(TEXT("보스 방에 들어왔다. 1.5초 뒤 플레이어가 문을 열면 보스가 등장한다."));

	After(1.5f, [this, Arena]
	{
		// 이 순간에 게임 스레드에서 일어나는 동기 로드(=화면이 멈추는 로드)를 센다.
		TArray<FString> SyncLoads;
		const FDelegateHandle Handle = FCoreDelegates::OnSyncLoadPackage.AddLambda([&SyncLoads](const FString& Package)
		{
			SyncLoads.Add(Package);
		});
		const double Start = FPlatformTime::Seconds();
		AStaticMeshActor* Boss = Arena->SpawnBoss();
		const double Ms = (FPlatformTime::Seconds() - Start) * 1000.0;
		FCoreDelegates::OnSyncLoadPackage.Remove(Handle);

		BugHunt::Note(FString::Printf(TEXT("보스 등장에 걸린 시간 %.1f ms, 동기 로드 %d번 %s"), Ms, SyncLoads.Num(),
			SyncLoads.Num() > 0 ? *FString::Printf(TEXT("(%s)"), *FString::Join(SyncLoads, TEXT(", "))) : TEXT("")));
		BugHunt::ExpectTrue(TEXT("보스에 외형이 있다"), Boss && Boss->GetStaticMeshComponent()->GetStaticMesh() != nullptr);
		BugHunt::Expect(TEXT("게임 중(문을 연 순간) 동기 로드 횟수"), 0, SyncLoads.Num());
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 보스는 잘 나오는데, 문을 연 순간 게임이 잠깐 멈춥니다(히치). 이 PC 에서는 몇 ms 지만 큰 에셋, 느린 디스크, 콘솔에서는 수백 ms 입니다.
// -trace=cpu,loadtime 으로 실행해 Unreal Insights 를 열면 Timing 창에 그 프레임의 LoadPackage 가, Asset Loading Insights 에 어떤 에셋인지 나옵니다.
// 콘솔 obj list class=StaticMesh 를 문 열기 전후로 쳐 보세요. 콘솔 명령 log LogStreaming Verbose 도 로드를 로그로 보여 줍니다.
#pragma endregion

#pragma region 힌트 2
// LoadSynchronous / LoadObject / StaticLoadObject 는 에셋이 메모리에 없으면 디스크에서 다 읽을 때까지 게임 스레드를 세웁니다.
// 소프트 레퍼런스(TSoftObjectPtr)로 둔 것은 잘했습니다. 문제는 "언제" 불러오느냐입니다. 보스 방에 들어온 때부터 문을 열 때까지 1.5초가 있습니다.
#pragma endregion

#pragma region 힌트 3
// 미리, 비동기로 불러 둡니다. BeginPlay 에서:
//     LoadHandle = UAssetManager::GetStreamableManager().RequestAsyncLoad(BossMesh.ToSoftObjectPath());  // TSharedPtr<FStreamableHandle> 멤버
// SpawnBoss 에서는 BossMesh.Get() (아직 안 왔으면 LoadHandle->WaitUntilComplete() 가 최후의 수단).
// 핸들을 들고 있어야 GC 가 불러 둔 에셋을 치우지 않습니다. 레벨 전체라면 로딩 화면 동안 미리 불러 두는 것이 보통입니다.
#pragma endregion
