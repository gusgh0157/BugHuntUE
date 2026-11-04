#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "BugHuntAnim.generated.h"

class UAnimInstance;
class UAnimMontage;
class UAnimSequenceBase;
class USkeletalMeshComponent;

// 9부(애니메이션) 랩 공용. 이 파일에는 버그가 없다.
// 엔진 기본 콘텐츠의 튜토리얼 캐릭터(TutorialTPP)를 쓰고, 몽타주와 노티파이는 에셋 대신 코드로 만든다.
// 애님 블루프린트 없이 기본 UAnimInstance 를 쓰므로, 몽타주는 재생·노티파이·종료 이벤트까지 정상으로 돌지만
// 화면의 포즈는 바뀌지 않는다(슬롯 노드가 없음). 랩은 포즈가 아니라 노티파이 횟수와 상태값으로 판정한다.
UCLASS(Abstract, NotPlaceable)
class ABH_AnimCharacter : public AActor
{
	GENERATED_BODY()

public:
	ABH_AnimCharacter();

	UAnimInstance* GetAnim() const;

	// 노티파이가 부른다. EventName 은 노티파이를 만들 때 정한 이름.
	virtual void OnAnimEvent(FName EventName) {}
	virtual void OnAnimStateBegin(FName EventName) {}
	virtual void OnAnimStateEnd(FName EventName) {}

	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TObjectPtr<USkeletalMeshComponent> Mesh;
};

// 지정한 시각에 한 번 OnAnimEvent 를 부르는 노티파이 (에디터의 "Add Notify"에 해당)
UCLASS(NotPlaceable)
class UBH_AnimNotify_Event : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override { return EventName.ToString(); }

	UPROPERTY()
	FName EventName;
};

// 구간의 시작과 끝에 OnAnimStateBegin/End 를 부르는 노티파이 스테이트 (에디터의 "Add Notify State"에 해당)
UCLASS(NotPlaceable)
class UBH_AnimNotifyState_Event : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override { return EventName.ToString(); }

	UPROPERTY()
	FName EventName;
};

namespace BugHunt
{
	// /Engine/Tutorial/SubEditors/TutorialAssets/Character/ 의 애니메이션 (Tutorial_Idle, Tutorial_Walk_Fwd)
	UAnimSequenceBase* LoadTutorialAnim(const TCHAR* Name);

	// Anim 하나로 DefaultSlot 몽타주를 만든다 (블렌드 인/아웃 0.25초). GC 되지 않게 UPROPERTY 로 들고 있을 것.
	UAnimMontage* MakeMontage(UAnimSequenceBase* Anim);

	// Time 초에 노티파이를 단다. Duration > 0 이면 [Time, Time+Duration] 노티파이 스테이트.
	void AddNotify(UAnimMontage* Montage, float Time, FName EventName, float Duration = 0.f);
}
