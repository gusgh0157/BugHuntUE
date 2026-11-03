#include "Labs/Lab08_Patrol.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"
#include "VisualLogger/VisualLogger.h"

DEFINE_LOG_CATEGORY(LogBugHuntPatrol);

ABH_Patroller::ABH_Patroller()
{
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	if (Cylinder.Succeeded())
	{
		Mesh->SetStaticMesh(Cylinder.Object);
		Mesh->SetRelativeScale3D(FVector(0.6f, 0.6f, 1.8f));   // 지름 60, 높이 180 (중심 기준 ±90)
	}
}

void ABH_Patroller::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Waypoints.IsEmpty())
	{
		return;
	}

	const FVector Location = GetActorLocation();
	const FVector Target = Waypoints[TargetIndex];

	// 바닥을 따라 걷는다 (위아래로는 움직이지 않음).
	const FVector Direction = (Target - Location).GetSafeNormal2D();
	SetActorLocation(Location + Direction * Speed * DeltaSeconds);

	const float Distance = FVector::Dist(GetActorLocation(), Target);

	UE_VLOG_LOCATION(this, LogBugHuntPatrol, Log, GetActorLocation(), 10.f, FColor::Green, TEXT("순찰병"));
	UE_VLOG_LOCATION(this, LogBugHuntPatrol, Log, Target, AcceptRadius, FColor::Yellow, TEXT("웨이포인트 %d"), TargetIndex);
	UE_VLOG_SEGMENT(this, LogBugHuntPatrol, Log, GetActorLocation(), Target, FColor::Red, TEXT("거리 %.1f"), Distance);
	UE_VLOG(this, LogBugHuntPatrol, Log, TEXT("목표 %d, 거리 %.1f, 반경 %.1f"), TargetIndex, Distance, AcceptRadius);

	if (Distance < AcceptRadius)
	{
		++WaypointsReached;
		TargetIndex = (TargetIndex + 1) % Waypoints.Num();
		UE_LOG(LogBugHuntPatrol, Log, TEXT("  웨이포인트 도착 -> 다음 목표 %d"), TargetIndex);
	}
}

void ABH_Lab08::StartLab()
{
	ABH_Patroller* Guard = SpawnLabActor<ABH_Patroller>(FVector(0.f, 0.f, ABH_Patroller::HalfHeight));
	Guard->Waypoints = {
		FVector(400.f, 0.f, 0.f),
		FVector(400.f, 400.f, 0.f),
		FVector(0.f, 400.f, 0.f),
		FVector(0.f, 0.f, 0.f),
	};
	BugHunt::Note(TEXT("순찰병이 한 변 400인 사각형을 돈다. 속도 400 → 5초면 웨이포인트 4개 이상."));

	After(5.f, [this, Guard]
	{
		BugHunt::ExpectTrue(FString::Printf(TEXT("5초 동안 지난 웨이포인트 수 >= 4 (실제: %d)"), Guard->WaypointsReached),
			Guard->WaypointsReached >= 4);
		Guard->SetActorTickEnabled(false);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 매 프레임 도는 코드라 F10 으로 따라가기 힘듭니다. Visual Logger 로 "녹화"해서 되감아 보세요.
//   1) 에디터 메뉴 Tools → Debug → Visual Logger 를 열고 Start(녹화) 버튼을 누른다.
//   2) PIE 실행 → 콘솔에 BugHunt.Lab 8 → 5초 기다린 뒤 Stop.
//   3) 타임라인에서 BH_Patroller 줄을 클릭하고 시간 막대를 끌어 보세요. 뷰포트에 초록 점(순찰병),
//      노란 원(웨이포인트, 반경), 빨간 선(남은 거리)이 그려지고, 오른쪽에 그 프레임의 로그가 나옵니다.
#pragma endregion

#pragma region 힌트 2
// 순찰병이 웨이포인트 바로 근처에서 앞뒤로 떨고 있는 프레임을 찾으세요. 그때 로그의 "거리"는 얼마인가요?
// 가로로는 거의 겹쳐 있는데 거리가 반경보다 큽니다. 뷰포트를 옆에서 보면 초록 점과 노란 원의 높이(Z)가 같나요?
#pragma endregion

#pragma region 힌트 3
// 액터 위치는 몸 중심(바닥 + HalfHeight), 웨이포인트는 바닥입니다. 3D 거리는 절대 90 아래로 내려가지 않습니다.
// 바닥을 따라 걷는 캐릭터의 도착 판정은 높이를 빼고 해야 합니다. (FVector::Dist2D 등)
#pragma endregion
