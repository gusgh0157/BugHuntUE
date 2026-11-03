#include "Labs/Lab14_Explosion.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/AudioComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

ABH_ExplosiveBarrel::ABH_ExplosiveBarrel()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));

	static ConstructorHelpers::FObjectFinder<USoundBase> Ping(TEXT("/Engine/EngineSounds/1kSineTonePing.1kSineTonePing"));
	ExplosionSound = Ping.Object;
}

UAudioComponent* ABH_ExplosiveBarrel::Explode()
{
	UE_LOG(LogBugHunt, Log, TEXT("  %s 폭발!"), *GetName());
	UAudioComponent* Audio = UGameplayStatics::SpawnSoundAttached(ExplosionSound, RootComponent);
	Destroy();
	return Audio;
}

void ABH_Lab14::StartLab()
{
	ABH_ExplosiveBarrel* Barrel = SpawnLabActor<ABH_ExplosiveBarrel>(FVector(300.f, 0.f, 50.f));

	const bool bHasAudio = GEngine && GEngine->UseSound() && GetWorld()->GetAudioDeviceRaw() != nullptr;
	UE_LOG(LogBugHunt, Log, TEXT("  오디오: UseSound %d, 메인 장치 %p, 월드 장치 %p"), GEngine && GEngine->UseSound(),
		GEngine ? GEngine->GetMainAudioDeviceRaw() : nullptr, GetWorld()->GetAudioDeviceRaw());
	if (!bHasAudio || !Barrel->ExplosionSound)
	{
		BugHunt::Skip(TEXT("랩 14"), TEXT("오디오 장치가 없습니다 (-nosound 로 실행했거나 출력 장치가 없음)."));
		FinishLab();
		return;
	}

	// 검사 환경 고정: 엔진은 1초 이하의 짧은 소리가 들을 수 없는 위치(리스너 밖)면 컴포넌트를 아예 만들지 않는다.
	// 카메라 위치나 헤드리스 실행에 결과가 흔들리지 않도록 이 최적화를 끈다. (버그와는 상관없음)
	if (IConsoleVariable* DistanceOpt = IConsoleManager::Get().FindConsoleVariable(TEXT("au.SoundDistanceOptimizationLength")))
	{
		DistanceOpt->Set(0.f);
	}

	const float Duration = Barrel->ExplosionSound->GetDuration();
	BugHunt::Note(FString::Printf(TEXT("폭발음 길이 %.2f초. 0.1초 뒤에도 들리고 있어야 한다."), Duration));

	UAudioComponent* RawAudio = Barrel->Explode();
	BugHunt::Note(FString::Printf(TEXT("폭발음 컴포넌트: %s"), RawAudio ? *RawAudio->GetPathName() : TEXT("없음")));
	TWeakObjectPtr<UAudioComponent> Audio = RawAudio;

	After(0.1f, [this, Audio]
	{
		const bool bStillPlaying = Audio.IsValid() && Audio->IsPlaying();
		UE_LOG(LogBugHunt, Log, TEXT("  0.1초 뒤: 컴포넌트 %s, 재생 중 %s"),
			Audio.IsValid() ? TEXT("살아 있음") : TEXT("사라짐"), bStillPlaying ? TEXT("예") : TEXT("아니오"));
		BugHunt::ExpectTrue(TEXT("0.1초 뒤에도 폭발음이 재생 중"), bStillPlaying);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 오디오 컴포넌트는 만들어졌는데 0.1초 뒤에 사라졌습니다. 누가 지웠을까요?
// Explode 의 Destroy() 줄에 중단점을 걸고, 넘기기 전에 조사식에 Audio->GetOwner() 와 Audio->GetAttachParent() 를 넣어 보세요.
// 콘솔: au.Debug.Sounds 1 (화면에 지금 재생 중인 소리 목록), stat soundwaves, au.3dVisualize.Enabled 1 (소리 위치 표시)
#pragma endregion

#pragma region 힌트 2
// SpawnSoundAttached 는 오디오 컴포넌트를 "붙인 대상의 액터"에 만듭니다. 액터가 파괴되면 그 액터의 컴포넌트도 함께 파괴되고,
// 컴포넌트가 파괴되면 재생 중이던 소리도 멈춥니다. Destroy() 다음 프레임부터는 소리가 없습니다.
#pragma endregion

#pragma region 힌트 3
// 액터보다 오래 살아야 하는 소리는 액터에 붙이지 않습니다. 그 자리에서 울리고 끝나는 소리는 어느 함수로 재생할까요?
// (UGameplayStatics::SpawnSoundAtLocation. 또는 액터를 바로 지우지 말고 숨긴 뒤 SetLifeSpan)
// 함정 하나 더: SpawnSoundAtLocation(this, ...) 처럼 첫 인자(WorldContextObject)에 곧 사라질 액터를 넘기면,
// 엔진은 그 액터를 컴포넌트의 주인으로 삼습니다. 고친 뒤에도 로그의 "폭발음 컴포넌트" 경로에 누가 들어 있는지 확인하세요.
#pragma endregion
