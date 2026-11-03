#include "Labs/Lab35_Footsteps.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/PlayerController.h"

ABH_Walker::ABH_Walker()
{
	// 최적화: NPC가 많아서, 화면에 안 보이는 NPC는 애니메이션을 돌리지 않게 했다.
	Mesh->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
}

void ABH_Walker::BeginPlay()
{
	Super::BeginPlay();

	// 에디터라면 걷기 애니메이션 에셋에 노티파이 4개를 달아 두는 일. 여기선 코드로 만든다.
	WalkMontage = BugHunt::MakeMontage(BugHunt::LoadTutorialAnim(TEXT("Tutorial_Walk_Fwd")));
	const float Length = WalkMontage->GetPlayLength();
	for (const float Fraction : { 0.125f, 0.375f, 0.625f, 0.875f })
	{
		BugHunt::AddNotify(WalkMontage, Length * Fraction, TEXT("Footstep"));
	}
}

float ABH_Walker::Walk()
{
	return GetAnim()->Montage_Play(WalkMontage);
}

void ABH_Walker::OnAnimEvent(FName EventName)
{
	if (EventName == TEXT("Footstep"))
	{
		++Footsteps;
		UE_LOG(LogBugHunt, Log, TEXT("  프레임 %llu: 발자국 %d (소리 → 근처 적이 들음)"), GFrameCounter, Footsteps);
	}
}

void ABH_Lab35::StartLab()
{
	// 플레이어 카메라 뒤쪽 4m 에서 NPC가 걸어간다. (플레이어는 못 보지만 적 AI는 발소리를 들어야 한다)
	FVector ViewLocation = FVector::ZeroVector;
	FRotator ViewRotation = FRotator::ZeroRotator;
	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		PC->GetPlayerViewPoint(ViewLocation, ViewRotation);
	}
	const FVector Behind = ViewLocation - ViewRotation.Vector() * 400.f;

	ABH_Walker* Walker = SpawnLabActor<ABH_Walker>(Behind);
	Walker->Mesh->SetCastShadow(false);   // 실습 조건 고정: 그림자도 화면에 안 걸리게

	const float Length = Walker->Walk();
	BugHunt::Note(FString::Printf(TEXT("카메라 뒤에서 걷기 몽타주 재생 (%.2f초, 발자국 노티파이 4개)"), Length));

	After(Length + 0.5f, [this, Walker]
	{
		UAnimInstance* Anim = Walker->GetAnim();
		BugHunt::Note(FString::Printf(TEXT("몽타주 아직 재생 중: %s, 위치 %.2f초"),
			Anim->Montage_IsPlaying(Walker->WalkMontage) ? TEXT("예") : TEXT("아니오"),
			Anim->Montage_GetPosition(Walker->WalkMontage)));
		BugHunt::Expect(TEXT("걷기 한 번의 발자국 수"), 4, Walker->Footsteps);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 발자국이 "적게"가 아니라 0번입니다. OnAnimEvent 에 중단점을 걸면 한 번도 안 멈춥니다.
// 그럼 몽타주는 돌고 있나요? 로그의 "몽타주 아직 재생 중 / 위치" 를 보세요. 재생 시간이 지났는데 위치가 0 이라면?
// 에디터에서 해 보면: 카메라를 돌려 NPC를 화면에 넣는 순간 발자국이 찍히기 시작합니다.
#pragma endregion

#pragma region 힌트 2
// 노티파이는 애니메이션이 "진행될 때" 지나간 구간에서 나옵니다. 애니메이션이 진행되지 않으면 노티파이도 없습니다.
// 그럼 애니메이션은 언제 진행되나요? USkeletalMeshComponent::TickComponent → TickPose 를 부르는 조건을 따라가 보세요.
// (조사식: Walker->Mesh->ShouldTickPose(), Walker->Mesh->bRecentlyRendered)
// 헤드리스 실행(-nullrhi)이나 데디케이티드 서버에서는 무엇이 "렌더됨"으로 처리될까요?
#pragma endregion

#pragma region 힌트 3
// VisibilityBasedAnimTickOption 은 "화면에 안 그려지면 애니메이션을 얼마나 아낄지" 입니다.
//     AlwaysTickPoseAndRefreshBones  항상 진행 + 본 갱신 (ACharacter 의 기본값)
//     AlwaysTickPose                 항상 진행, 본 갱신은 보일 때만 (스켈레탈 메시 컴포넌트 기본값)
//     OnlyTickMontagesWhenNotRendered 안 보이면 몽타주만 진행 (몽타주 노티파이는 나옴)
//     OnlyTickPoseWhenRendered       안 보이면 아무것도 진행 안 함 → 노티파이, 몽타주 종료 이벤트 전부 멈춤
// 게임 판정(발소리 감지, 공격 판정)에 쓰는 노티파이가 있다면 서버와 화면 밖에서도 진행되어야 합니다.
#pragma endregion
