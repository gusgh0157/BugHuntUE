#include "Labs/Lab45_Minimap.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Kismet/GameplayStatics.h"

int32 ABH_MapMarker::SearchCount = 0;

ABH_MapHero::ABH_MapHero()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

ABH_MapMarker::ABH_MapMarker()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

ABH_MapHero* ABH_MapMarker::FindHero(const UWorld* World)
{
	++SearchCount;
	TArray<AActor*> Found;
	UGameplayStatics::GetAllActorsOfClass(World, ABH_MapHero::StaticClass(), Found);
	return Found.Num() > 0 ? Cast<ABH_MapHero>(Found[0]) : nullptr;
}

void ABH_MapMarker::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (const ABH_MapHero* Hero = FindHero(GetWorld()))
	{
		const float Distance = FVector::Dist(Hero->GetActorLocation(), GetActorLocation());
		Opacity = FMath::Clamp(1.f - Distance / 5000.f, 0.2f, 1.f);
	}
}

void ABH_Lab45::StartLab()
{
	BugHunt::SetFixedFrameRate(60.f);
	ABH_MapMarker::SearchCount = 0;

	SpawnLabActor<ABH_MapHero>();
	const int32 NumMarkers = BugHunt::CaseInt(200, 100, 400);
	TArray<ABH_MapMarker*> Markers;
	for (int32 i = 0; i < NumMarkers; ++i)
	{
		Markers.Add(SpawnLabActor<ABH_MapMarker>(FVector(100.f * (i % 50), 100.f * (i / 50), 0.f)));
	}
	BugHunt::Note(FString::Printf(TEXT("미니맵 아이콘 %d개. 1초(60프레임) 동안 돌려 본다."), NumMarkers));

	const double StartTime = FPlatformTime::Seconds();
	After(1.0f, [this, Markers, NumMarkers, StartTime]
	{
		const int32 Searches = ABH_MapMarker::SearchCount;
		BugHunt::Note(FString::Printf(TEXT("1초 동안 액터 전체 검색 %d번 (실제 시간 %.0f ms)"), Searches, (FPlatformTime::Seconds() - StartTime) * 1000.0));
		BugHunt::ExpectTrue(FString::Printf(TEXT("액터 전체 검색은 아이콘마다 한 번 이하 (%d번 ≤ %d)"), Searches, NumMarkers), Searches <= NumMarkers);
		BugHunt::ExpectTrue(TEXT("가까운 아이콘은 진하게 (Opacity > 0.9)"), Markers[0]->Opacity > 0.9f);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 결과는 맞는데 검색 횟수가 너무 많습니다. PIE 에서 콘솔 stat game, stat unit 을 켜고 BugHunt.Lab 45 를 실행해 보세요. Game 시간이 얼마나 뛰나요?
// 어디서 시간을 쓰는지 보려면 명령줄에 -trace=cpu 를 붙여 실행하고 Unreal Insights 로 열어 Timing 창에서 긴 프레임을 고르세요.
// (stat startfile / stat stopfile 로 남긴 .ue4stats 파일도 Insights 나 Session Frontend 에서 열 수 있습니다.)
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// FindHero 는 월드의 액터를 전부 훑습니다. 아이콘 N 개가 매 프레임 한 번씩 부르면 프레임마다 N × (액터 수) 번 일을 합니다.
// 플레이어는 게임 중에 바뀌지 않습니다. 한 번 찾은 것을 기억해 두면 되지 않을까요?
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// BeginPlay 에서 한 번 찾아 약한 포인터로 기억합니다:

// [JA] 結果は合っていますが、検索回数が多すぎます。PIE でコンソールの stat game、stat unit をオンにして BugHunt.Lab 45 を実行してみてください。Game の時間はどれくらい跳ね上がりますか？
// [JA] どこで時間を使っているか見るには、コマンドラインに -trace=cpu を付けて実行し、Unreal Insights で開いて Timing ウィンドウで長いフレームを選んでください。
// [JA] （stat startfile / stat stopfile で残した .ue4stats ファイルも Insights や Session Frontend で開けます。）
//     UPROPERTY() TWeakObjectPtr<ABH_MapHero> Hero;      // (헤더)
//     void BeginPlay() { Super::BeginPlay(); Hero = FindHero(GetWorld()); }
//     Tick: if (const ABH_MapHero* H = Hero.Get()) { ... }
// FindComponentByClass, GetComponents, TActorIterator 도 같습니다. Tick 안에서 무언가를 "찾는" 코드는 의심하세요.
// 실제 게임에서는 플레이어 쪽이 등록하는 방식(서브시스템에 자신을 알림)이나 GetPlayerPawn(this, 0) 처럼 이미 기억된 것을 씁니다.

// [JA] FindHero はワールドのアクターをすべて走査します。アイコン N 個が毎フレーム 1 回ずつ呼ぶと、フレームごとに N ×（アクター数）回の処理をします。
// [JA] プレイヤーはゲーム中に変わりません。一度見つけたものを覚えておけばよいのではないでしょうか？
#pragma endregion

// [JA] BeginPlay で一度見つけて、弱いポインタで覚えておきます:
// [JA]     UPROPERTY() TWeakObjectPtr<ABH_MapHero> Hero;      // （ヘッダー）
// [JA]     void BeginPlay() { Super::BeginPlay(); Hero = FindHero(GetWorld()); }
// [JA]     Tick: if (const ABH_MapHero* H = Hero.Get()) { ... }
// [JA] FindComponentByClass、GetComponents、TActorIterator も同じです。Tick の中で何かを「探す」コードは疑ってください。
// [JA] 実際のゲームでは、プレイヤー側が登録する方式（サブシステムに自分を知らせる）や、GetPlayerPawn(this, 0) のようにすでに覚えてあるものを使います。
