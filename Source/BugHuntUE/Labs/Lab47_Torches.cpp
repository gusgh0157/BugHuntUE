#include "Labs/Lab47_Torches.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

int32 ABH_Torch::TickCalls = 0;

ABH_Torch::ABH_Torch()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ABH_Torch::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	++TickCalls;

	FlickerTimer += DeltaSeconds;
	if (FlickerTimer >= 0.5f)
	{
		FlickerTimer -= 0.5f;
		Flicker();
	}
}

void ABH_Torch::Flicker()
{
	++Flickers;
	SetActorScale3D(FVector(1.f, 1.f, 0.9f + 0.2f * (Flickers % 2)));
}

void ABH_Lab47::StartLab()
{
	BugHunt::SetFixedFrameRate(60.f);
	ABH_Torch::TickCalls = 0;

	const int32 NumTorches = BugHunt::CaseInt(300, 150, 600);
	TArray<ABH_Torch*> Torches;
	for (int32 i = 0; i < NumTorches; ++i)
	{
		Torches.Add(SpawnLabActor<ABH_Torch>(FVector(200.f * i, 0.f, 300.f)));
	}
	BugHunt::Note(FString::Printf(TEXT("성벽에 횃불 %d개. 2초(120프레임) 동안 0.5초마다 일렁여야 한다."), NumTorches));

	const int32 StartTicks = ABH_Torch::TickCalls;
	After(2.05f, [this, Torches, NumTorches, StartTicks]
	{
		const int32 Ticks = ABH_Torch::TickCalls - StartTicks;
		int32 MinFlickers = MAX_int32;
		int32 MaxFlickers = 0;
		for (const ABH_Torch* Torch : Torches)
		{
			MinFlickers = FMath::Min(MinFlickers, Torch->Flickers);
			MaxFlickers = FMath::Max(MaxFlickers, Torch->Flickers);
		}
		BugHunt::Note(FString::Printf(TEXT("횃불 Tick 호출 %d번, 일렁임 %d~%d번"), Ticks, MinFlickers, MaxFlickers));
		BugHunt::ExpectTrue(FString::Printf(TEXT("횃불마다 4번쯤 일렁였다 (%d~%d번, 3~5 이면 통과)"), MinFlickers, MaxFlickers), MinFlickers >= 3 && MaxFlickers <= 5);
		BugHunt::ExpectTrue(FString::Printf(TEXT("횃불 Tick 은 일렁임 1번에 1번 이하 (%d번 ≤ %d)"), Ticks, NumTorches * 5), Ticks <= NumTorches * 5);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 일렁임은 맞는데 Tick 이 너무 많이 불립니다. 콘솔 dumpticks 로 지금 등록된 틱 함수 목록과 개수를 보세요(랩 실행 중에).
// stat game 의 "Tick Time", stat unit 의 Game 도 같이 보세요. 횃불 하나는 가볍지만 수백 개가 매 프레임 불리면 어떨까요?
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 이 횃불이 실제로 하는 일은 0.5초에 한 번입니다. 나머지 Tick 은 시간을 더하고 비교만 합니다.
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// "매 프레임 할 일이 없는 액터는 Tick 을 끈다" 가 기본입니다. 주기적으로 할 일이 있으면 그 주기로만 불리게 합니다.
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 방법 1: 타이머. 생성자에서 PrimaryActorTick.bCanEverTick = false; 하고

// [JA] 揺らめきは合っていますが、Tick が呼ばれすぎています。コンソールの dumpticks で、今登録されている Tick 関数の一覧と数を見てください（ラボの実行中に）。
// [JA] stat game の "Tick Time"、stat unit の Game も一緒に見てください。たいまつ 1 本は軽いですが、数百本が毎フレーム呼ばれたらどうでしょう？
//     BeginPlay: GetWorldTimerManager().SetTimer(FlickerHandle, this, &ABH_Torch::Flicker, 0.5f, true);
// 방법 2: 틱 간격. PrimaryActorTick.TickInterval = 0.5f; (Tick 의 DeltaSeconds 가 0.5 가 된다)
// 수천 개라면 매니저 하나가 전부를 돌리거나(배열 순회가 액터별 Tick 보다 훨씬 쌈), Significance Manager 로 멀리 있는 것은 덜 자주 돌립니다.
#pragma endregion

// [JA] このたいまつが実際に仕事をするのは 0.5 秒に 1 回です。残りの Tick は時間を足して比べるだけです。
// [JA] 「毎フレームやることがないアクターは Tick をオフにする」が基本です。定期的にやることがあるなら、その周期でだけ呼ばれるようにします。

// [JA] 方法 1: タイマー。コンストラクタで PrimaryActorTick.bCanEverTick = false; にして
// [JA]     BeginPlay: GetWorldTimerManager().SetTimer(FlickerHandle, this, &ABH_Torch::Flicker, 0.5f, true);
// [JA] 方法 2: Tick 間隔。PrimaryActorTick.TickInterval = 0.5f;（Tick の DeltaSeconds が 0.5 になる）
// [JA] 数千本なら、マネージャー 1 つが全部を回すか（配列の走査はアクターごとの Tick よりずっと安い）、Significance Manager で遠くにあるものは回す頻度を下げます。
