#include "Labs/Lab02_ScoreHud.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 게임이 시작되면 점수 관리자가 점수판 HUD를 띄우고, HUD는 점수가 바뀔 때마다 글자를 갱신한다.

void UBH_Scoreboard::AddScore(const FString& Player, int32 Points)
{
	int32& Score = Scores.FindOrAdd(Player);
	Score += Points;
	OnScoreChanged.Broadcast(Player, Score);
}

int32 UBH_Scoreboard::ScoreOf(const FString& Player) const
{
	const int32* Score = Scores.Find(Player);
	return Score ? *Score : 0;
}

ABH_ScoreManager::ABH_ScoreManager()
{
	PrimaryActorTick.bCanEverTick = false;
}

void ABH_ScoreManager::BeginPlay()
{
	Super::BeginPlay();

	// 화면부터 띄우고
	FActorSpawnParameters Params;
	Params.Owner = this;
	Hud = GetWorld()->SpawnActor<ABH_ScoreHud>(ABH_ScoreHud::StaticClass(), FTransform::Identity, Params);

	// 점수판을 준비한다.
	Scoreboard = NewObject<UBH_Scoreboard>(this, TEXT("Scoreboard"));
}

void ABH_ScoreHud::BeginPlay()
{
	Super::BeginPlay();

	ABH_ScoreManager* Manager = CastChecked<ABH_ScoreManager>(GetOwner());
	Manager->GetScoreboard()->OnScoreChanged.AddUObject(this, &ABH_ScoreHud::HandleScoreChanged);
	DisplayedText = TEXT("점수 없음");
}

void ABH_ScoreHud::HandleScoreChanged(const FString& Player, int32 NewScore)
{
	DisplayedText = FString::Printf(TEXT("%s %d"), *Player, NewScore);
}

void ABH_Lab02::StartLab()
{
	ABH_ScoreManager* Manager = SpawnLabActor<ABH_ScoreManager>();

	Manager->GetScoreboard()->AddScore(TEXT("Kael"), 10);
	Manager->GetScoreboard()->AddScore(TEXT("Kael"), 20);

	BugHunt::Expect(TEXT("Kael 점수"), 30, Manager->GetScoreboard()->ScoreOf(TEXT("Kael")));
	BugHunt::ExpectTrue(FString::Printf(TEXT("HUD 표시 == \"Kael 30\" (실제: \"%s\")"), *Manager->Hud->DisplayedText),
		Manager->Hud->DisplayedText == TEXT("Kael 30"));
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 호출 스택을 보고 펼치세요)
// 멈춘 곳은 ABH_ScoreHud::BeginPlay 입니다. 이 줄에서 nullptr 인 것은 Manager 인가요,
// Manager->GetScoreboard() 의 결과인가요? 조사식 창에 Manager->Scoreboard 를 넣어 보세요.
#pragma endregion

#pragma region 힌트 2
// 호출 스택을 아래로 내려가 보세요. ABH_ScoreHud::BeginPlay 와 ABH_ScoreManager::BeginPlay 사이에
// UWorld::SpawnActor, AActor::FinishSpawning, AActor::DispatchBeginPlay 같은 엔진 프레임이 있습니다.
// 즉 HUD의 BeginPlay는 "나중에"가 아니라 SpawnActor 호출 안에서 바로 실행됐습니다.
// 매니저의 BeginPlay 프레임을 더블클릭하면 초록 화살표가 어느 줄을 가리키나요? 그때 Scoreboard는?
#pragma endregion

#pragma region 힌트 3
// 게임이 이미 시작된 월드에서 SpawnActor 를 하면, 생성자 → PostInitializeComponents → BeginPlay 가
// SpawnActor 가 돌아오기 전에 전부 실행됩니다.
// 다른 객체가 쓸 멤버는 언제 만들어 두는 게 안전할까요? (순서 바꾸기 / 생성자 / PostInitializeComponents)
#pragma endregion
