#include "Labs/Lab27_Knockback.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"

ABH_Brute::ABH_Brute()
{
	Body = CreateDefaultSubobject<UBoxComponent>(TEXT("Body"));
	Body->InitBoxExtent(FVector(40.f, 40.f, 90.f));
	Body->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	RootComponent = Body;
}

void ABH_Brute::Knockback(const FVector& Direction, float Distance)
{
	const FVector Target = GetActorLocation() + Direction * Distance;
	SetActorLocation(Target);
	UE_LOG(LogBugHunt, Log, TEXT("  넉백 %.0f cm → X=%.0f"), Distance, GetActorLocation().X);
}

ABH_CastleWall::ABH_CastleWall()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->InitBoxExtent(FVector(20.f, 500.f, 300.f));
	Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	RootComponent = Box;
}

void ABH_Lab27::StartLab()
{
	// 기본 맵의 바닥과 플레이어에서 떨어진 빈 공중에서 실험한다.
	const FVector Base(0.f, 3000.f, 2000.f);
	const float WallX = static_cast<float>(BugHunt::CaseInt(300, 200, 400));
	const float Distance = static_cast<float>(BugHunt::CaseInt(500, 450, 900));
	SpawnLabActor<ABH_CastleWall>(Base + FVector(WallX, 0.f, 90.f));
	ABH_Brute* Brute = SpawnLabActor<ABH_Brute>(Base + FVector(0.f, 0.f, 90.f));
	BugHunt::Note(FString::Printf(TEXT("거한 뒤 %.0f cm 에 성벽(두께 40cm). 망치로 쳐서 %.0f cm 밀어낸다."), WallX, Distance));

	After(0.1f, [this, Brute, WallX, Distance]
	{
		Brute->Knockback(FVector(1.f, 0.f, 0.f), Distance);
		const float WallFront = WallX - 20.f;
		const float BruteBack = Brute->GetActorLocation().X + 40.f;
		BugHunt::ExpectTrue(FString::Printf(TEXT("거한이 성벽을 넘지 않았다 (거한 뒷면 X=%.0f ≤ 성벽 앞면 X=%.0f)"), BruteBack, WallFront), BruteBack <= WallFront + 1.f);
		BugHunt::ExpectTrue(TEXT("거한이 성벽에 닿을 때까지 밀렸다 (5cm 이내)"), BruteBack >= WallFront - 5.f && BruteBack <= WallFront + 1.f);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 거한이 성벽 너머에 서 있습니다. 둘 다 BlockAll 인데도요. show Collision 을 켜고 넉백 직후 일시 정지해서 보세요.
// SetActorLocation 에 F12 로 들어가 매개변수 목록을 보세요. 생략한 매개변수의 기본값은 무엇인가요?
#pragma endregion

#pragma region 힌트 2
// 충돌 설정(Block)은 "움직이는 방법"이 충돌을 검사할 때만 의미가 있습니다. SetActorLocation(위치) 은 기본으로 순간이동입니다:
// 목적지에 그냥 놓고, 가는 길에 무엇이 있었는지는 보지 않습니다. 물리 시뮬레이션 중이 아니므로 물리가 밀어내 주지도 않습니다.
#pragma endregion

#pragma region 힌트 3
//     FHitResult Hit;
//     SetActorLocation(Target, /*bSweep*/ true, &Hit);   // 가는 길을 쓸어서(스윕) 막히면 그 앞에서 멈춘다
//     if (Hit.bBlockingHit) { /* 벽에 부딪힘: 스턴, 소리 */ }
// AddActorWorldOffset, SetWorldLocation 도 같은 bSweep 매개변수가 있습니다. 캐릭터라면 LaunchCharacter 나 CharacterMovement 가
// 스윕을 대신 해 줍니다. 스윕은 루트 컴포넌트의 모양으로만 합니다(자식 컴포넌트의 충돌은 보지 않음).
#pragma endregion
