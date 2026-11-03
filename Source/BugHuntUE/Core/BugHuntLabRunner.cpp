// 콘솔 명령:
//   BugHunt.List      랩 목록
//   BugHunt.Lab N     랩 N 실행 (PIE 또는 -game 실행 중에)
//   BugHunt.Lab N S   랩 N을 숨은 입력(시드 S)으로 실행. S가 0이면 공개 입력
// 명령줄:
//   -BugHuntLab=N     월드가 시작되면 랩 N을 자동 실행
//   -BugHuntExit      랩이 끝나면 종료 (자동 검증용)
//   -BugHuntSeed=S    숨은 입력(시드 S)으로 실행 (채점기가 쓴다)

#include "Core/BugHuntLabRunner.h"
#include "BugHuntUE.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntLab.h"
#include "Core/BugHuntReport.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/CommandLine.h"
#include "TimerManager.h"

#include "Labs/Lab01_Pickup.h"
#include "Labs/Lab02_ScoreHud.h"
#include "Labs/Lab03_Cleanup.h"
#include "Labs/Lab04_QuestLog.h"
#include "Labs/Lab05_CombatLog.h"
#include "Labs/Lab06_Poison.h"
#include "Labs/Lab07_TickOrder.h"
#include "Labs/Lab08_Patrol.h"
#include "Labs/Lab09_Archer.h"
#include "Labs/Lab10_Balance.h"
#include "Labs/Lab11_RepNotify.h"
#include "Labs/Lab12_Chest.h"
#include "Labs/Lab13_FireSound.h"
#include "Labs/Lab14_Explosion.h"
#include "Labs/Lab15_AlarmBell.h"
#include "Labs/Lab16_Highlight.h"
#include "Labs/Lab17_ItemIcons.h"
#include "Labs/Lab18_TeamColor.h"
#include "Labs/Lab19_Footsteps.h"
#include "Labs/Lab20_ComboSwing.h"
#include "Labs/Lab21_HitWindow.h"
#include "Labs/Lab22_ThreatTable.h"
#include "Labs/Lab23_Turret.h"
#include "Labs/Lab24_PotionBelt.h"
#include "Labs/Lab25_DialogueTree.h"
#include "Labs/Lab26_Elevator.h"
#include "Labs/Lab27_Pathfinding.h"
#include "Labs/Lab28_AutoSave.h"
#include "Labs/Lab29_WaveRewards.h"
#include "Labs/Lab30_PriceTable.h"
#include "Labs/Lab31_BountyBoard.h"
#include "Labs/Lab32_TurnOrder.h"
#include "Labs/Lab33_FogOfWar.h"
#include "Labs/Lab34_LootBag.h"
#include "Labs/Lab35_QuestBoard.h"
#include "Labs/Lab36_Dash.h"
#include "Labs/Lab37_IceSlide.h"
#include "Labs/Lab38_RoundStats.h"
#include "Labs/Lab39_Sniper.h"
#include "Labs/Lab40_PressurePlate.h"
#include "Labs/Lab41_Knockback.h"
#include "Labs/Lab42_Catapult.h"
#include "Labs/Lab43_GuildVault.h"
#include "Labs/Lab44_ScoreFeed.h"
#include "Labs/Lab45_Minimap.h"
#include "Labs/Lab46_AimPreview.h"
#include "Labs/Lab47_Torches.h"
#include "Labs/Lab48_BossEntrance.h"

namespace
{
struct FLabEntry
{
	const TCHAR* Title;
	const TCHAR* Symptom;
	UClass* (*GetClass)();
};

const FLabEntry GLabs[] = {
	// 1부: 크래시 추적
	{ TEXT("회복 아이템"),   TEXT("펫이 아이템을 줍자 게임이 죽는다"),           [] { return ABH_Lab01::StaticClass(); } },
	{ TEXT("점수판 HUD"),    TEXT("게임 시작하자마자 죽는다"),                   [] { return ABH_Lab02::StaticClass(); } },
	{ TEXT("죽은 적 정리"),  TEXT("Ensure가 뜨고 죽은 적이 남아 있다"),          [] { return ABH_Lab03::StaticClass(); } },
	{ TEXT("퀘스트 로그"),   TEXT("두 번째 퀘스트를 받을 때 죽는다 (가끔 안 죽음)"), [] { return ABH_Lab04::StaticClass(); } },
	// 2부: 흐름 추적 (크래시 없음)
	{ TEXT("전투 기록 창"),  TEXT("창을 다시 열면 기록이 두 줄씩 찍힌다"),       [] { return ABH_Lab05::StaticClass(); } },
	{ TEXT("독 정화"),       TEXT("정화했는데 독 피해가 계속 들어온다"),         [] { return ABH_Lab06::StaticClass(); } },
	{ TEXT("HP 바"),         TEXT("HP 바가 한 프레임씩 늦다"),                   [] { return ABH_Lab07::StaticClass(); } },
	{ TEXT("순찰병"),        TEXT("순찰병이 첫 웨이포인트에서 떨고만 있다"),     [] { return ABH_Lab08::StaticClass(); } },
	{ TEXT("궁수"),          TEXT("쓰러진 허수아비로 점수를 계속 번다"),         [] { return ABH_Lab09::StaticClass(); } },
	{ TEXT("밸런스 패치"),   TEXT("HP를 150으로 올렸는데 게임에선 100이다"),     [] { return ABH_Lab10::StaticClass(); } },
	// 3부: 멀티플레이
	{ TEXT("호스트 HP 바"),  TEXT("리슨 서버 호스트 화면만 HP 바가 안 바뀐다"),  [] { return ABH_Lab11::StaticClass(); } },
	{ TEXT("보물 상자"),     TEXT("클라이언트가 연 상자의 골드가 서버에 없다"),  [] { return ABH_Lab12::StaticClass(); } },
	// 4부: 사운드
	{ TEXT("블래스터 발사음"), TEXT("발사음이 섬광보다 늦게 난다 (첫 발, 그리고 가끔)"), [] { return ABH_Lab13::StaticClass(); } },
	{ TEXT("폭발 통"),       TEXT("폭발음이 터지자마자 끊긴다"),                 [] { return ABH_Lab14::StaticClass(); } },
	{ TEXT("경보 종"),       TEXT("호스트만 종소리를 듣는다"),                   [] { return ABH_Lab15::StaticClass(); } },
	// 5부: 이미지 / 렌더링
	{ TEXT("상자 강조"),     TEXT("마우스를 올려도 상자가 빨개지지 않는다"),     [] { return ABH_Lab16::StaticClass(); } },
	{ TEXT("인벤토리 아이콘"), TEXT("스크롤하면 엉뚱한 아이템 아이콘이 뜬다"),   [] { return ABH_Lab17::StaticClass(); } },
	{ TEXT("팀 색"),         TEXT("팀 색이 기획서보다 밝고 물 빠져 보인다"),     [] { return ABH_Lab18::StaticClass(); } },
	// 6부: 애니메이션
	{ TEXT("발자국"),        TEXT("카메라 밖 NPC의 발소리를 적이 못 듣는다"),     [] { return ABH_Lab19::StaticClass(); } },
	{ TEXT("연타 콤보"),     TEXT("연타하면 공격 중인데 움직여지고 콤보가 더 오른다"), [] { return ABH_Lab20::StaticClass(); } },
	{ TEXT("히트박스"),      TEXT("공격이 끊긴 뒤에도 칼에 닿으면 맞는다"),       [] { return ABH_Lab21::StaticClass(); } },
	// 7부: 잘못된 포인터
	{ TEXT("어그로 표"),     TEXT("새로 온 영웅이 죽은 영웅의 어그로를 물려받는다"), [] { return ABH_Lab22::StaticClass(); } },
	{ TEXT("포탑"),          TEXT("부순 포탑이 계속 쏜다"),                       [] { return ABH_Lab23::StaticClass(); } },
	{ TEXT("물약 벨트"),     TEXT("전리품을 주운 뒤로 물약이 줄지 않는다"),       [] { return ABH_Lab24::StaticClass(); } },
	{ TEXT("대화 트리"),     TEXT("대화가 끝나도 대화 노드가 메모리에 남는다"),   [] { return ABH_Lab25::StaticClass(); } },
	// 8부: 멈춤 (무한루프와 데드락)
	{ TEXT("엘리베이터"),    TEXT("엘리베이터를 부르면 게임이 굳는다"),           [] { return ABH_Lab26::StaticClass(); } },
	{ TEXT("길찾기"),        TEXT("끊긴 다리 너머로 보내면 게임이 굳는다"),       [] { return ABH_Lab27::StaticClass(); } },
	{ TEXT("자동 저장"),     TEXT("자동 저장할 때 게임이 굳는다"),                 [] { return ABH_Lab28::StaticClass(); } },
	// 9부: 컨테이너
	{ TEXT("웨이브 보상"),   TEXT("보상을 받으면 크래시한다"),                     [] { return ABH_Lab29::StaticClass(); } },
	{ TEXT("상점 시세표"),   TEXT("모르는 물건을 팔면 크래시한다"),               [] { return ABH_Lab30::StaticClass(); } },
	{ TEXT("현상금 게시판"), TEXT("새 몬스터를 잡으면 현상금 집계가 틀린다"),     [] { return ABH_Lab31::StaticClass(); } },
	{ TEXT("턴 순서"),       TEXT("유닛이 쓰러지면 턴 순서가 뒤섞인다"),           [] { return ABH_Lab32::StaticClass(); } },
	{ TEXT("전장의 안개"),   TEXT("같은 칸이 여러 번 밝혀진다"),                   [] { return ABH_Lab33::StaticClass(); } },
	{ TEXT("전리품 가방"),   TEXT("시간이 지나면 가방 속 아이템이 사라진다"),     [] { return ABH_Lab34::StaticClass(); } },
	{ TEXT("퀘스트 게시판"), TEXT("클라이언트의 퀘스트 진행도가 0에서 멈춘다"),   [] { return ABH_Lab35::StaticClass(); } },
	// 10부: 프레임레이트와 초기화
	{ TEXT("돌진"),          TEXT("프레임레이트에 따라 돌진 거리가 다르다"),       [] { return ABH_Lab36::StaticClass(); } },
	{ TEXT("얼음 미끄럼"),   TEXT("프레임레이트에 따라 미끄러지는 거리가 다르다"), [] { return ABH_Lab37::StaticClass(); } },
	{ TEXT("라운드 점수판"), TEXT("새 라운드에 지난 라운드 점수가 남아 있다"),     [] { return ABH_Lab38::StaticClass(); } },
	// 11부: 물리
	{ TEXT("저격탄"),        TEXT("탄환이 얇은 철판을 뚫고 지나간다"),             [] { return ABH_Lab39::StaticClass(); } },
	{ TEXT("압력판"),        TEXT("상자를 올려도 압력판이 눌리지 않는다"),         [] { return ABH_Lab40::StaticClass(); } },
	{ TEXT("넉백"),          TEXT("밀려난 적이 성벽 너머에 서 있다"),               [] { return ABH_Lab41::StaticClass(); } },
	{ TEXT("투석기"),        TEXT("투석기가 던진 바위가 그 자리에 떨어진다"),       [] { return ABH_Lab42::StaticClass(); } },
	// 12부: 데이터 레이스
	{ TEXT("길드 금고"),     TEXT("동시에 입금하면 골드가 모자란다"),               [] { return ABH_Lab43::StaticClass(); } },
	{ TEXT("하이라이트 점수"), TEXT("리플레이 분석이 끝나면 크래시한다"),          [] { return ABH_Lab44::StaticClass(); } },
	// 13부: 성능
	{ TEXT("미니맵"),        TEXT("아이콘이 많으면 게임이 느려진다"),               [] { return ABH_Lab45::StaticClass(); } },
	{ TEXT("조준선"),        TEXT("조준하는 동안 메모리 할당이 잦다"),             [] { return ABH_Lab46::StaticClass(); } },
	{ TEXT("횃불"),          TEXT("횃불이 많으면 게임이 느려진다"),                 [] { return ABH_Lab47::StaticClass(); } },
	{ TEXT("보스 등장"),     TEXT("보스가 나올 때 화면이 멈칫한다"),                 [] { return ABH_Lab48::StaticClass(); } },
};
constexpr int32 GLabCount = UE_ARRAY_COUNT(GLabs);

// 각 부의 첫 랩 번호 (채점표의 "부" 칸)
constexpr int32 GPartFirstLab[] = { 1, 5, 11, 13, 16, 19, 22, 26, 29, 36, 39, 43, 45 };

int32 PartOf(int32 Number)
{
	int32 Part = 0;
	for (int32 First : GPartFirstLab)
	{
		Part += Number >= First ? 1 : 0;
	}
	return Part;
}

// 서버 + 클라이언트 두 프로세스로 검사하는 랩
bool IsNetworkedLab(int32 Number)
{
	return Number == 11 || Number == 12 || Number == 15 || Number == 35;
}

// 실제 GPU 렌더링 결과를 읽어야 판정되는 랩. 지금은 없다 (README 5부 참고). 생기면 채점기가 SKIP 한다.
bool NeedsGPU(int32 /*Number*/)
{
	return false;
}

void ListLabs()
{
	UE_LOG(LogBugHunt, Display, TEXT("BugHunt UE 랩 목록 (실행: BugHunt.Lab 번호)"));
	for (int32 i = 0; i < GLabCount; ++i)
	{
		UE_LOG(LogBugHunt, Display, TEXT("  %2d. %s - %s"), i + 1, GLabs[i].Title, GLabs[i].Symptom);
	}
}

void RunLabImpl(UWorld* World, int32 Number, int32 Seed)
{
	if (Number < 1 || Number > GLabCount)
	{
		UE_LOG(LogBugHunt, Error, TEXT("랩 번호는 1~%d 입니다."), GLabCount);
		ListLabs();
		return;
	}
	if (!World || !World->IsGameWorld())
	{
		UE_LOG(LogBugHunt, Error, TEXT("게임 월드가 없습니다. 먼저 플레이(PIE)를 시작한 뒤 게임 창의 콘솔(~)에서 입력하세요."));
		return;
	}

	// 이전 랩과 그 랩이 만든 액터를 정리한다.
	for (TActorIterator<ABugHuntLab> It(World); It; ++It)
	{
		It->Destroy();
	}

	if (Seed >= 0)
	{
		BugHunt::SetCaseSeed(Seed);
	}
	const FLabEntry& Entry = GLabs[Number - 1];
	FActorSpawnParameters Params;
	Params.Name = MakeUniqueObjectName(World->GetCurrentLevel(), Entry.GetClass(), *FString::Printf(TEXT("BugHuntLab%02d"), Number));
	ABugHuntLab* Lab = World->SpawnActor<ABugHuntLab>(Entry.GetClass(), FTransform::Identity, Params);
	Lab->BeginLab(Number, Entry.Title);
}

FAutoConsoleCommandWithWorldAndArgs GLabCommand(
	TEXT("BugHunt.Lab"),
	TEXT("BugHunt.Lab N [시드] : 랩 N을 실행한다. (PIE 중에 입력) 시드를 주면 채점기처럼 숨은 입력으로, 0이면 공개 입력으로 돈다."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateLambda([](const TArray<FString>& Args, UWorld* World)
	{
		if (Args.Num() < 1)
		{
			ListLabs();
			return;
		}
		RunLabImpl(World, FCString::Atoi(*Args[0]), Args.Num() >= 2 ? FCString::Atoi(*Args[1]) : INDEX_NONE);
	}));

FAutoConsoleCommand GListCommand(
	TEXT("BugHunt.List"),
	TEXT("BugHunt 랩 목록을 Output Log에 출력한다."),
	FConsoleCommandDelegate::CreateLambda([] { ListLabs(); }));
}

void BugHunt::RunLab(UWorld* World, int32 Number)
{
	RunLabImpl(World, Number, INDEX_NONE);
}

TArray<BugHunt::FLabInfo> BugHunt::GetLabs()
{
	TArray<FLabInfo> Labs;
	for (int32 i = 0; i < GLabCount; ++i)
	{
		FLabInfo& Info = Labs.AddDefaulted_GetRef();
		Info.Number = i + 1;
		Info.Title = GLabs[i].Title;
		Info.Symptom = GLabs[i].Symptom;
		Info.Part = PartOf(Info.Number);
		Info.bNetworked = IsNetworkedLab(Info.Number);
		Info.bNeedsGPU = NeedsGPU(Info.Number);
	}
	return Labs;
}

bool UBugHuntAutoRunSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UBugHuntAutoRunSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// 자동 검증용: 서버가 먼저 끝나 접속이 끊기면 클라이언트도 종료한다.
	static bool bHookedNetFailure = false;
	if (!bHookedNetFailure && GEngine && FParse::Param(FCommandLine::Get(), TEXT("BugHuntExit")))
	{
		bHookedNetFailure = true;
		GEngine->OnNetworkFailure().AddLambda([](UWorld*, UNetDriver*, ENetworkFailure::Type, const FString&)
		{
			BugHunt::ExitIfAutomated();
		});
	}

	static bool bAlreadyRan = false;
	int32 Number = 0;
	if (bAlreadyRan || InWorld.GetNetMode() == NM_Client
		|| !FParse::Value(FCommandLine::Get(), TEXT("BugHuntLab="), Number))
	{
		return;
	}
	bAlreadyRan = true;

	// 월드가 한 프레임 돈 뒤에 시작한다.
	TWeakObjectPtr<UWorld> WeakWorld(&InWorld);
	InWorld.GetTimerManager().SetTimerForNextTick([WeakWorld, Number]
	{
		BugHunt::RunLab(WeakWorld.Get(), Number);
	});
}
