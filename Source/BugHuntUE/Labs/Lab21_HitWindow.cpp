#include "Labs/Lab21_HitWindow.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

void ABH_Knight::BeginPlay()
{
	Super::BeginPlay();

	// 에디터라면 베기 애니메이션 에셋에 노티파이를 달아 두는 일. 칼이 지나가는 구간 = 길이의 30% ~ 60%.
	SlashMontage = BugHunt::MakeMontage(BugHunt::LoadTutorialAnim(TEXT("Tutorial_Walk_Fwd")));
	const float Length = SlashMontage->GetPlayLength();
	BugHunt::AddNotify(SlashMontage, Length * 0.3f, TEXT("HitOn"));
	BugHunt::AddNotify(SlashMontage, Length * 0.6f, TEXT("HitOff"));
}

void ABH_Knight::Slash()
{
	GetAnim()->Montage_Play(SlashMontage);
}

void ABH_Knight::Stagger()
{
	UE_LOG(LogBugHunt, Log, TEXT("  프레임 %llu: 맞음 → 경직, 베기 중단"), GFrameCounter);
	GetAnim()->Montage_Stop(0.1f, SlashMontage);
}

void ABH_Knight::OnAnimEvent(FName EventName)
{
	if (EventName == TEXT("HitOn"))
	{
		bHitboxOn = true;
	}
	else if (EventName == TEXT("HitOff"))
	{
		bHitboxOn = false;
	}
	UE_LOG(LogBugHunt, Log, TEXT("  프레임 %llu: %s → 히트박스 %s"), GFrameCounter, *EventName.ToString(), bHitboxOn ? TEXT("켬") : TEXT("끔"));
}

void ABH_Lab21::StartLab()
{
	ABH_Knight* Knight = SpawnLabActor<ABH_Knight>(FVector(300.f, 0.f, 100.f));
	const float Length = Knight->SlashMontage->GetPlayLength();
	const float HitAt = Length * 0.45f;
	BugHunt::Note(FString::Printf(TEXT("베기 %.2f초, 히트박스 구간 %.2f~%.2f초. %.2f초에 적에게 맞아 경직"),
		Length, Length * 0.3f, Length * 0.6f, HitAt));

	Knight->Slash();
	After(HitAt - 0.05f, [Knight]
	{
		BugHunt::ExpectTrue(TEXT("칼이 지나가는 중: 히트박스 켜짐"), Knight->bHitboxOn);
	});
	After(HitAt, [Knight] { Knight->Stagger(); });

	After(HitAt + 1.0f, [this, Knight]
	{
		BugHunt::Note(FString::Printf(TEXT("경직 1초 뒤 베기 재생 중: %s"),
			Knight->GetAnim()->Montage_IsPlaying(Knight->SlashMontage) ? TEXT("예") : TEXT("아니오")));
		BugHunt::ExpectTrue(TEXT("베기가 끊긴 뒤: 히트박스 꺼짐 (가만히 서 있는 칼에 적이 닿아도 안 맞음)"), !Knight->bHitboxOn);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 로그에서 HitOn 다음에 HitOff 가 찍혔나요? 경직은 히트박스 구간의 어디쯤에서 일어났나요?
// OnAnimEvent 에 중단점을 걸고, 경직 뒤에 한 번이라도 더 멈추는지 보세요.
#pragma endregion

#pragma region 힌트 2
// 노티파이는 몽타주가 그 시각을 "지나갈 때"만 옵니다. 몽타주가 그 전에 멈추면 HitOff 는 영영 오지 않습니다.
// 켜는 일과 끄는 일을 서로 다른 시각의 노티파이 두 개에 맡겼습니다. 둘 중 하나만 오는 경우는 어떤 때인가요?
// (경직, 사망, 회피, 다른 몽타주 재생, 렉으로 프레임이 크게 뛰어 몽타주가 끝나 버림...)
#pragma endregion

#pragma region 힌트 3
// 구간에 걸친 상태는 노티파이 "스테이트"(UAnimNotifyState)로 만듭니다. NotifyBegin 이 불렸다면 NotifyEnd 는
// 몽타주가 끝나든 끊기든 반드시 불립니다. 이 프로젝트의 BugHunt::AddNotify 에 Duration 을 주면 스테이트가 됩니다.
// 또는 몽타주가 끝나거나 끊길 때(OnMontageBlendingOut / OnMontageEnded) 히트박스를 끄는 안전장치를 둡니다.
#pragma endregion
