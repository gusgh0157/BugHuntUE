#include "Labs/Lab44_AutoSave.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntHang.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Async/Async.h"

FString ABH_SaveSystem::Describe(const ABH_SaveHero* Hero)
{
	return FString::Printf(TEXT("%s Lv%d Gold%d"), *Hero->HeroName, Hero->Level, Hero->Gold);
}

void ABH_SaveSystem::AutoSave(ABH_SaveHero* Hero)
{
	const float Seconds = WriteSeconds;
	TFuture<FString> Done = Async(EAsyncExecution::ThreadPool, [Hero, Seconds]
	{
		// 1) 영웅 상태 읽기. 액터는 게임 스레드에서만 만져야 하므로 게임 스레드에 부탁하고 결과를 기다린다.
		TPromise<FString> Promise;
		TFuture<FString> Snapshot = Promise.GetFuture();
		AsyncTask(ENamedThreads::GameThread, [Hero, Promise = MoveTemp(Promise)]() mutable
		{
			Promise.SetValue(Describe(Hero));
		});
		const FString Text = Snapshot.Get();

		// 2) 디스크에 쓰기 (느리다)
		FPlatformProcess::Sleep(Seconds);
		return Text;
	});

	// 저장 도중에 게임 상태가 바뀌면 안 되니까 저장이 끝날 때까지 기다린다.
	if (BugHunt::WaitWithBudget(Done, TEXT("자동 저장 끝나기를 기다림")))   // = Done.Get() (실습용: 시간 제한이 있다)
	{
		OnSaved(Done.Get());
	}
}

void ABH_SaveSystem::OnSaved(const FString& Text)
{
	bSaved = true;
	SavedText = Text;
	UE_LOG(LogBugHunt, Log, TEXT("  %.2f초: 저장 완료 \"%s\""), GetWorld()->GetTimeSeconds(), *Text);
}

void ABH_Lab44::StartLab()
{
	ABH_SaveHero* Hero = SpawnLabActor<ABH_SaveHero>();
	ABH_SaveSystem* Save = SpawnLabActor<ABH_SaveSystem>();
	Hero->Level = BugHunt::CaseInt(7, 2, 60);
	Hero->Gold = BugHunt::CaseInt(120, 10, 9999);
	Save->WriteSeconds = BugHunt::CaseFloat(0.3f, 0.2f, 0.5f);

	const FString Expected = ABH_SaveSystem::Describe(Hero);
	BugHunt::Note(FString::Printf(TEXT("자동 저장 시작: \"%s\". 디스크 쓰기 %.1f초. 저장 직후 상자를 열어 골드가 늘어난다."), *Expected, Save->WriteSeconds));

	const double Start = FPlatformTime::Seconds();
	Save->AutoSave(Hero);
	const double BlockedMs = (FPlatformTime::Seconds() - Start) * 1000.0;

	Hero->Gold += 50;   // 저장을 요청한 "뒤"의 변화. 저장 파일에 들어가면 안 된다.

	After(Save->WriteSeconds + 1.0f, [this, Save, Expected, BlockedMs]
	{
		BugHunt::ExpectTrue(FString::Printf(TEXT("자동 저장이 게임을 멈춘 시간 < 50 ms (실제: %.0f ms)"), BlockedMs), BlockedMs < 50.0);
		BugHunt::ExpectTrue(TEXT("저장 완료"), Save->bSaved);
		BugHunt::ExpectTrue(FString::Printf(TEXT("저장 내용 == \"%s\" (실제: \"%s\")"), *Expected, *Save->SavedText), Save->SavedText == Expected);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// BugHunt.HangBudgetSeconds 30 → BugHunt.Lab 44 → 굳은 동안 VS 에서 모두 중단(Ctrl+Alt+Break).
// 디버그 → 창 → 병렬 스택 (Ctrl+Shift+D, S) 을 열면 모든 스레드의 호출 스택이 한 그림에 나옵니다.
// GameThread 는 무엇을 기다리고 있나요? 그 결과를 만들 작업 스레드(이름에 Worker/Pool)는 무엇을 기다리고 있나요?
// 스레드 창(Ctrl+Alt+H)에서 두 스레드를 번갈아 더블클릭하며 각자의 호출 스택을 보세요.
#pragma endregion

#pragma region 힌트 2
// 작업 스레드: "게임 스레드야, 영웅 상태 좀 읽어 줘" (AsyncTask GameThread) 를 보내고 Snapshot.Get() 으로 기다림.
// 게임 스레드: 작업 스레드가 끝나기를 Done 에서 기다림. 게임 스레드에 보낸 일은 게임 스레드가 다음 프레임에 처리하는데,
// 게임 스레드는 지금 기다리느라 다음 프레임으로 못 갑니다. 서로가 서로를 기다리는 데드락입니다.
// 이 랩은 2초 뒤 기다리기를 포기하지만, 진짜 Done.Get() 은 영원히 기다립니다.
// 데드락이 없어도 문제가 하나 더 있습니다: 게임 스레드가 디스크 쓰기 시간만큼 멈춥니다 (프레임 히치).
#pragma endregion

#pragma region 힌트 3
// 규칙 1: 게임 스레드에서 작업 스레드 결과를 동기로 기다리지 않는다. 끝나면 게임 스레드로 "알려 달라"고 한다.
// 규칙 2: 작업 스레드가 쓸 게임 데이터는 보내기 전에 게임 스레드에서 복사해 둔다.
//     const FString Text = Describe(Hero);                       // 지금 순간의 상태 (게임 스레드)
//     TWeakObjectPtr<ABH_SaveSystem> WeakThis(this);
//     Async(EAsyncExecution::ThreadPool, [Text, Seconds, WeakThis]
//     {
//         FPlatformProcess::Sleep(Seconds);                         // 디스크 쓰기
//         AsyncTask(ENamedThreads::GameThread, [Text, WeakThis]
//         {
//             if (ABH_SaveSystem* Self = WeakThis.Get()) { Self->OnSaved(Text); }
//         });
//     });
// (작업 스레드에서 Hero->Gold 를 직접 읽는 것은 데이터 경쟁입니다. 저장 파일에 +50 된 골드가 들어갈 수도 있습니다.)
#pragma endregion
