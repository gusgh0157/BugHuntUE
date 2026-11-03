#include "Labs/Lab42_Catapult.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"

ABH_Boulder::ABH_Boulder()
{
	Ball = CreateDefaultSubobject<USphereComponent>(TEXT("Ball"));
	Ball->InitSphereRadius(30.f);
	Ball->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Ball->SetSimulatePhysics(true);
	RootComponent = Ball;
}

ABH_Catapult::ABH_Catapult()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ABH_Catapult::Swing(ABH_Boulder* InBoulder, const FVector& InArmVelocity)
{
	Carried = InBoulder;
	ArmTip = InBoulder->GetActorLocation();
	ArmVelocity = InArmVelocity;
}

void ABH_Catapult::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (!Carried)
	{
		return;
	}
	// 팔 끝을 움직이고, 얹힌 바위를 팔 끝에 붙여 둔다.
	ArmTip += ArmVelocity * DeltaSeconds;
	Carried->SetActorLocation(ArmTip);
}

void ABH_Catapult::Release()
{
	UE_LOG(LogBugHunt, Log, TEXT("  발사! 바위 속도 (%.0f, %.0f, %.0f)"),
		Carried->Ball->GetPhysicsLinearVelocity().X, Carried->Ball->GetPhysicsLinearVelocity().Y, Carried->Ball->GetPhysicsLinearVelocity().Z);
	Carried = nullptr;
}

void ABH_Lab42::StartLab()
{
	BugHunt::SetFixedFrameRate(60.f);

	const float ArmSpeed = static_cast<float>(BugHunt::CaseInt(1000, 600, 1500));
	ABH_Catapult* Catapult = SpawnLabActor<ABH_Catapult>();
	// 기본 맵의 바닥과 플레이어에서 떨어진 빈 공중에서 실험한다.
	ABH_Boulder* Boulder = SpawnLabActor<ABH_Boulder>(FVector(0.f, 3000.f, 2000.f));
	BugHunt::Note(FString::Printf(TEXT("투석기 팔이 초속 %.0f cm 로 앞(+X)과 위(+Z)로 0.5초 휘두른 뒤 바위를 놓는다."), ArmSpeed));

	const FVector ArmVelocity(ArmSpeed, 0.f, ArmSpeed);
	After(0.1f, [Catapult, Boulder, ArmVelocity] { Catapult->Swing(Boulder, ArmVelocity); });
	After(0.6f, [Catapult] { Catapult->Release(); });
	After(0.65f, [this, Boulder, ArmVelocity]
	{
		const FVector V = Boulder->Ball->GetPhysicsLinearVelocity();
		BugHunt::Note(FString::Printf(TEXT("놓은 직후 바위 속도 (%.0f, %.0f, %.0f) cm/s"), V.X, V.Y, V.Z));
		BugHunt::ExpectTrue(FString::Printf(TEXT("앞(+X)으로 날아간다: 속도 X %.0f ≈ 팔 속도 %.0f (±25%%)"), V.X, ArmVelocity.X), FMath::Abs(V.X - ArmVelocity.X) <= ArmVelocity.X * 0.25f);
		BugHunt::ExpectTrue(FString::Printf(TEXT("위(+Z)로 날아오른다: 속도 Z %.0f > 0"), V.Z), V.Z > 0.f);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 휘두르는 동안 바위는 팔 끝을 잘 따라갔는데, 놓는 순간 그 자리에서 뚝 떨어집니다.
// Release 의 로그와 Tick 에 추적점 {Carried->Ball->GetPhysicsLinearVelocity()} 를 걸어 보세요. 휘두르는 동안 물리 엔진이 아는 바위 속도는 얼마인가요?
// Chaos Visual Debugger 로 녹화하면 프레임마다 물리 쪽 위치와 속도를 볼 수 있습니다.
#pragma endregion

#pragma region 힌트 2
// 물리 시뮬레이션 중인 물체에 SetActorLocation 을 하면 "그 위치로 옮겨 놓기"만 합니다. 물리 엔진의 속도는 그대로입니다(중력이 쌓은 아래 방향 속도까지).
// 매 프레임 옮겨 놓아도 물리 엔진은 바위가 움직이고 있다고 생각하지 않습니다. 그래서 놓으면 물리가 아는 속도로 떨어집니다.
// 반대로, 물리 물체를 Tick 에서 옮겨 놓으면 다른 물체와 부딪혀도 제대로 밀어내지 못하고 덜덜 떠는 문제도 같은 원인입니다.
#pragma endregion

#pragma region 힌트 3
// 물리 물체는 물리의 방법으로 움직입니다. 들고 다니는 동안 속도로 움직이게 하거나:
//     Carried->Ball->SetPhysicsLinearVelocity(ArmVelocity);
// 들고 있는 동안은 시뮬레이션을 끄고(SetSimulatePhysics(false), 또는 PhysicsHandle 로 끌기) 놓을 때 켜면서 속도를 넣어 줍니다:
//     Ball->SetSimulatePhysics(true);  Ball->SetPhysicsLinearVelocity(ArmVelocity);
// 힘으로 밀려면 AddForce / AddImpulse. 어느 쪽이든 "놓는 순간의 속도"를 물리 엔진이 알아야 합니다.
#pragma endregion
