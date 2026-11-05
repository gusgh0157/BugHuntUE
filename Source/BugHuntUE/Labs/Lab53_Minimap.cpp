#include "Labs/Lab53_Minimap.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Kismet/GameplayStatics.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab53Text
{
BH_TEXT(Start,    "미니맵 아이콘 %d개. 1초(60프레임) 동안 돌려 본다.", "ミニマップのアイコン %d 個。1 秒（60 フレーム）間動かしてみる。");
BH_TEXT(Searched, "1초 동안 액터 전체 검색 %d번 (실제 시간 %.0f ms)", "1 秒間でアクター全体の検索 %d 回（実時間 %.0f ms）");
BH_TEXT(PerIcon,  "액터 전체 검색은 아이콘마다 한 번 이하 (%d번 ≤ %d)", "アクター全体の検索はアイコンごとに 1 回以下（%d 回 ≤ %d）");
BH_TEXT(Near,     "가까운 아이콘은 진하게 (Opacity > 0.9)", "近いアイコンは濃く（Opacity > 0.9）");
}

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

void ABH_Lab53::StartLab()
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
	BugHunt::Note(BugHunt::Fmt<Lab53Text::Start>(NumMarkers));

	const double StartTime = FPlatformTime::Seconds();
	After(1.0f, [this, Markers, NumMarkers, StartTime]
	{
		const int32 Searches = ABH_MapMarker::SearchCount;
		BugHunt::Note(BugHunt::Fmt<Lab53Text::Searched>(Searches, (FPlatformTime::Seconds() - StartTime) * 1000.0));
		BugHunt::ExpectTrue(BugHunt::Fmt<Lab53Text::PerIcon>(Searches, NumMarkers), Searches <= NumMarkers);
		BugHunt::ExpectTrue(BugHunt::Tr<Lab53Text::Near>(), Markers[0]->Opacity > 0.9f);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 결과는 맞는데 검색 횟수가 너무 많습니다. PIE 에서 콘솔 stat game, stat unit 을 켜고 BugHunt.Lab 53 를 실행해 보세요. Game 시간이 얼마나 뛰나요?
// 어디서 시간을 쓰는지 보려면 명령줄에 -trace=cpu 를 붙여 실행하고 Unreal Insights 로 열어 Timing 창에서 긴 프레임을 고르세요.
// (Insights 가 여는 것은 -trace 로 남긴 .utrace 입니다. stat startfile / stat stopfile 의 .ue4stats 는 비추천된 Session Frontend 의 Profiler 용입니다.)

// [JA] 結果は合っていますが、検索回数が多すぎます。PIE でコンソールの stat game、stat unit をオンにして BugHunt.Lab 53 を実行してみてください。Game の時間はどれくらい跳ね上がりますか？
// [JA] どこで時間を使っているか見るには、コマンドラインに -trace=cpu を付けて実行し、Unreal Insights で開いて Timing ウィンドウで長いフレームを選んでください。
// [JA] （Insights が開くのは -trace で残した .utrace です。stat startfile / stat stopfile の .ue4stats は非推奨の Session Frontend の Profiler 用です。）
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// FindHero 는 월드의 액터를 전부 훑습니다. 아이콘 N 개가 매 프레임 한 번씩 부르면 프레임마다 N × (액터 수) 번 일을 합니다.
// 플레이어는 게임 중에 바뀌지 않습니다. 한 번 찾은 것을 기억해 두면 되지 않을까요?

// [JA] FindHero はワールドのアクターをすべて走査します。アイコン N 個が毎フレーム 1 回ずつ呼ぶと、フレームごとに N ×（アクター数）回の処理をします。
// [JA] プレイヤーはゲーム中に変わりません。一度見つけたものを覚えておけばよいのではないでしょうか？
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// BeginPlay 에서 한 번 찾아 약한 포인터로 기억합니다:
//     UPROPERTY() TWeakObjectPtr<ABH_MapHero> Hero;      // (헤더)
//     void BeginPlay() { Super::BeginPlay(); Hero = FindHero(GetWorld()); }
//     Tick: if (const ABH_MapHero* H = Hero.Get()) { ... }
// FindComponentByClass, GetComponents, TActorIterator 도 같습니다. Tick 안에서 무언가를 "찾는" 코드는 의심하세요.
// 실제 게임에서는 플레이어 쪽이 등록하는 방식(서브시스템에 자신을 알림)이나 GetPlayerPawn(this, 0) 처럼 이미 기억된 것을 씁니다.

// [JA] BeginPlay で一度見つけて、弱いポインタで覚えておきます:
// [JA]     UPROPERTY() TWeakObjectPtr<ABH_MapHero> Hero;      // （ヘッダー）
// [JA]     void BeginPlay() { Super::BeginPlay(); Hero = FindHero(GetWorld()); }
// [JA]     Tick: if (const ABH_MapHero* H = Hero.Get()) { ... }
// [JA] FindComponentByClass、GetComponents、TActorIterator も同じです。Tick の中で何かを「探す」コードは疑ってください。
// [JA] 実際のゲームでは、プレイヤー側が登録する方式（サブシステムに自分を知らせる）や、GetPlayerPawn(this, 0) のようにすでに覚えてあるものを使います。
#pragma endregion
