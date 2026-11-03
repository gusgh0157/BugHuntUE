#pragma once

#include "CoreMinimal.h"
#include "Core/BugHuntLab.h"
#include "Lab13_FireSound.generated.h"

class USoundBase;

// 블래스터: 쏘면 총구 섬광과 같은 프레임에 발사음이 나야 한다.
// 발사음은 메모리를 아끼려고 소프트 레퍼런스로 들고 있다.
UCLASS(NotPlaceable)
class ABH_Blaster : public AActor
{
	GENERATED_BODY()

public:
	ABH_Blaster();

	void Fire();

	UPROPERTY(EditAnywhere, Category = "BugHunt")
	TSoftObjectPtr<USoundBase> FireSound;

	// 발사마다 "총구 섬광 → 발사음"까지 걸린 프레임 수
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TArray<int32> SoundDelayFrames;

	// 발사마다 "총구 섬광 → 발사음"까지 걸린 시간(ms)
	UPROPERTY(VisibleAnywhere, Category = "BugHunt")
	TArray<float> SoundDelayMs;

private:
	void PlayFireSound(USoundBase* Sound, uint64 FireFrame, double FireTime);
};

UCLASS(NotPlaceable)
class ABH_Lab13 : public ABugHuntLab
{
	GENERATED_BODY()

protected:
	virtual void StartLab() override;
};
