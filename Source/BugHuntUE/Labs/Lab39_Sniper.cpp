#include "Labs/Lab39_Sniper.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"

ABH_SniperRound::ABH_SniperRound()
{
	Ball = CreateDefaultSubobject<USphereComponent>(TEXT("Ball"));
	Ball->InitSphereRadius(3.f);
	Ball->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Ball->SetSimulatePhysics(true);
	Ball->SetEnableGravity(false);
	Ball->SetNotifyRigidBodyCollision(true);     // "Simulation Generates Hit Events"
	Ball->OnComponentHit.AddDynamic(this, &ABH_SniperRound::OnHit);
	RootComponent = Ball;
}

void ABH_SniperRound::Fire(float Speed)
{
	Ball->SetPhysicsLinearVelocity(FVector(Speed, 0.f, 0.f));
}

void ABH_SniperRound::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	++Hits;
	UE_LOG(LogBugHunt, Log, TEXT("  탄환 명중: %s (X=%.0f)"), *GetNameSafe(OtherActor), GetActorLocation().X);
}

ABH_SteelPlate::ABH_SteelPlate()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->InitBoxExtent(FVector(2.f, 200.f, 200.f));      // 두께 4cm
	Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	RootComponent = Box;
}

void ABH_Lab39::StartLab()
{
	BugHunt::SetFixedFrameRate(60.f);

	// 기본 맵의 바닥과 플레이어에서 떨어진 빈 공중에서 실험한다.
	const FVector Base(0.f, 3000.f, 2000.f);
	const float Speed = static_cast<float>(BugHunt::CaseInt(30000, 15000, 60000));
	// 철판은 약 10m 앞. (어느 물리 단계에서도 탄환이 우연히 철판 안에 놓이지 않도록 단계 사이에 둔다)
	const float Step = Speed / 60.f;
	const float PlateX = (FMath::FloorToFloat(1000.f / Step) + 0.5f) * Step;
	SpawnLabActor<ABH_SteelPlate>(Base + FVector(PlateX, 0.f, 0.f));
	ABH_SniperRound* Round = SpawnLabActor<ABH_SniperRound>(Base);
	BugHunt::Note(FString::Printf(TEXT("%.1fm 앞 철판(두께 4cm)에 탄환을 초속 %.0f m 로 쏜다. 60fps."), PlateX / 100.f, Speed / 100.f));

	After(0.1f, [Round, Speed] { Round->Fire(Speed); });
	After(0.8f, [this, Round, PlateX]
	{
		BugHunt::ExpectTrue(TEXT("탄환이 철판에 맞았다 (Hit 이벤트)"), Round->Hits > 0);
		BugHunt::ExpectTrue(FString::Printf(TEXT("탄환이 철판 앞에 있다 (X=%.0f < %.0f)"), Round->GetActorLocation().X, PlateX), Round->GetActorLocation().X < PlateX);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 탄환이 철판을 뚫고 지나갔고 Hit 이벤트도 없습니다. PIE 에서 콘솔 show Collision, 그리고 p.Chaos.DebugDraw.Enabled 1 을 켜고 실행해 보세요.
// 더 자세히 보려면 Tools → Debug → Chaos Visual Debugger 에서 녹화를 켜고 실행한 뒤 프레임을 한 칸씩 넘겨 보세요. 탄환은 프레임마다 어디에 있나요?
#pragma endregion

#pragma region 힌트 2
// 물리는 프레임마다 "지금 위치"에서 겹침을 검사합니다. 한 프레임(1/60초)에 탄환이 가는 거리와 철판 두께를 비교해 보세요.
// 두 위치 사이를 지나간 것은 기본 설정으로는 보이지 않습니다. 이것을 터널링이라고 합니다.
#pragma endregion

#pragma region 힌트 3
// 빠르고 작은 물체에는 연속 충돌 검출(CCD)을 켭니다. 생성자에서:
//     Ball->SetUseCCD(true);                // = 디테일 패널 Collision → Use CCD
// CCD 는 비쌉니다. 모든 물체에 켜지 말고 빠른 것에만. 총알이라면 물리 대신 라인 트레이스(히트스캔)나
// 이동 거리만큼 스윕하는 ProjectileMovementComponent 가 더 흔한 답입니다. 서브스테핑(Project Settings → Physics → Substepping)도
// 한 단계에 가는 거리를 줄이지만, 이 속도에서는 모자랍니다.
#pragma endregion
