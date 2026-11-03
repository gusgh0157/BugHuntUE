#include "Labs/Lab03_Cleanup.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

void ABH_WaveManager::CleanupDeadEnemies()
{
	for (ABH_Enemy* Enemy : Enemies)
	{
		if (Enemy->Health <= 0)
		{
			UE_LOG(LogBugHunt, Log, TEXT("  제거: %s"), *Enemy->DisplayName);
			Enemies.Remove(Enemy);
			Enemy->Destroy();
			++Removed;
		}
	}
}

void ABH_Lab03::StartLab()
{
	ABH_WaveManager* Wave = SpawnLabActor<ABH_WaveManager>();

	struct FSpec { const TCHAR* Name; int32 Health; };
	const FSpec Specs[] = {
		{ TEXT("Slime"), 0 },
		{ TEXT("Bat"),   0 },
		{ TEXT("Orc"),   50 },
		{ TEXT("Wolf"),  0 },
		{ TEXT("Golem"), 80 },
	};
	for (const FSpec& Spec : Specs)
	{
		ABH_Enemy* Enemy = SpawnLabActor<ABH_Enemy>();
		Enemy->DisplayName = Spec.Name;
		Enemy->Health = Spec.Health;
		Wave->Enemies.Add(Enemy);
	}

	BugHunt::Note(TEXT("웨이브 종료. Slime, Bat, Wolf 가 죽었다 (HP 0)."));
	Wave->CleanupDeadEnemies();

	int32 DeadLeft = 0;
	for (ABH_Enemy* Enemy : Wave->Enemies)
	{
		BugHunt::Note(FString::Printf(TEXT("남은 적: %s (HP %d)"), *Enemy->DisplayName, Enemy->Health));
		if (Enemy->Health <= 0)
		{
			++DeadLeft;
		}
	}
	BugHunt::Expect(TEXT("남은 적 수"), 2, Wave->Enemies.Num());
	BugHunt::Expect(TEXT("목록에 남은 죽은 적"), 0, DeadLeft);
	BugHunt::Expect(TEXT("제거한 적 수"), 3, Wave->Removed);
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 Output Log를 보고 펼치세요)
// 이 랩은 프로그램이 죽지 않습니다. 대신 Output Log에 빨간 글씨로
//     Ensure condition failed: ... Array has changed during ranged-for iteration!
// 과 호출 스택이 찍힙니다. 디버거가 붙어 있으면 그 자리에서 멈춥니다(중단점처럼).
// 스택 맨 위의 TCheckedPointerIterator 등 엔진 템플릿은 건너뛰고 첫 ABH_ 함수를 찾으세요.
#pragma endregion

#pragma region 힌트 2
// CleanupDeadEnemies 의 for 줄에 중단점을 걸고, 조사식에 Enemies 와 Enemy 를 넣고 F10 으로 따라가세요.
// 첫 바퀴에서 Slime 을 Remove 하면 Enemies 는 어떻게 바뀌나요? 두 번째 바퀴의 Enemy 는 누구인가요?
// Bat 은 언제 검사되나요?
#pragma endregion

#pragma region 힌트 3
// 범위 기반 for 는 "처음 배열" 기준으로 칸을 하나씩 넘깁니다. 도는 중에 원소를 빼면 뒤 원소가 당겨져서
// 하나를 건너뛰고, 끝에서는 이미 지난 원소를 다시 봅니다.
// 순회하면서 지우려면 뒤에서부터 인덱스로 돌거나(RemoveAt), RemoveAll 을 쓰세요.
#pragma endregion
