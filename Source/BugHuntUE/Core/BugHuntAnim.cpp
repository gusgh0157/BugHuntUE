#include "Core/BugHuntAnim.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimSequenceBase.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "UObject/ConstructorHelpers.h"

ABH_AnimCharacter::ABH_AnimCharacter()
{
	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> Tpp(TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/TutorialTPP.TutorialTPP"));
	Mesh->SetSkeletalMesh(Tpp.Object);
	Mesh->SetAnimationMode(EAnimationMode::AnimationBlueprint);
	Mesh->SetAnimInstanceClass(UAnimInstance::StaticClass());
}

UAnimInstance* ABH_AnimCharacter::GetAnim() const
{
	return Mesh->GetAnimInstance();
}

void UBH_AnimNotify_Event::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	if (ABH_AnimCharacter* Owner = Cast<ABH_AnimCharacter>(MeshComp->GetOwner()))
	{
		Owner->OnAnimEvent(EventName);
	}
}

void UBH_AnimNotifyState_Event::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);
	if (ABH_AnimCharacter* Owner = Cast<ABH_AnimCharacter>(MeshComp->GetOwner()))
	{
		Owner->OnAnimStateBegin(EventName);
	}
}

void UBH_AnimNotifyState_Event::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);
	if (ABH_AnimCharacter* Owner = Cast<ABH_AnimCharacter>(MeshComp->GetOwner()))
	{
		Owner->OnAnimStateEnd(EventName);
	}
}

namespace BugHunt
{
UAnimSequenceBase* LoadTutorialAnim(const TCHAR* Name)
{
	const FString Path = FString::Printf(TEXT("/Engine/Tutorial/SubEditors/TutorialAssets/Character/%s.%s"), Name, Name);
	return LoadObject<UAnimSequenceBase>(nullptr, *Path);
}

UAnimMontage* MakeMontage(UAnimSequenceBase* Anim)
{
	return UAnimMontage::CreateSlotAnimationAsDynamicMontage(Anim, TEXT("DefaultSlot"), 0.25f, 0.25f);
}

void AddNotify(UAnimMontage* Montage, float Time, FName EventName, float Duration)
{
#if WITH_EDITORONLY_DATA
	if (Montage->AnimNotifyTracks.Num() == 0)
	{
		Montage->AnimNotifyTracks.Add(FAnimNotifyTrack(TEXT("1"), FLinearColor::White));
	}
#endif

	FAnimNotifyEvent& Event = Montage->Notifies.AddDefaulted_GetRef();
	Event.NotifyName = EventName;
	Event.TrackIndex = 0;
	Event.Link(Montage, Time);
	Event.TriggerTimeOffset = GetTriggerTimeOffsetForType(Montage->CalculateOffsetForNotify(Time));

	if (Duration > 0.f)
	{
		UBH_AnimNotifyState_Event* State = NewObject<UBH_AnimNotifyState_Event>(Montage);
		State->EventName = EventName;
		Event.NotifyStateClass = State;
		Event.SetDuration(Duration);
		Event.EndLink.Link(Montage, Time + Duration);
		Event.EndTriggerTimeOffset = GetTriggerTimeOffsetForType(Montage->CalculateOffsetForNotify(Time + Duration));
	}
	else
	{
		UBH_AnimNotify_Event* Notify = NewObject<UBH_AnimNotify_Event>(Montage);
		Notify->EventName = EventName;
		Event.Notify = Notify;
	}
	Montage->RefreshCacheData();
}
}
