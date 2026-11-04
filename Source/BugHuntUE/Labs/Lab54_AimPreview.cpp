#include "Labs/Lab54_AimPreview.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab54Text
{
BH_TEXT(Start,     "조준선 점 %d개를 매 프레임 계산한다. 1초(60프레임) 동안.", "照準線の点 %d 個を毎フレーム計算する。1 秒（60 フレーム）間。");
BH_TEXT(Allocs,    "%d프레임 동안 배열 메모리 할당 %d번 (프레임당 %.1f번)", "%d フレームの間に配列のメモリ確保 %d 回（1 フレームあたり %.1f 回）");
BH_TEXT(NumPoints, "조준선 점 수", "照準線の点の数");
BH_TEXT(PerFrame,  "배열 메모리 할당은 프레임당 한 번 이하 (%d번 ≤ %d프레임)", "配列のメモリ確保は 1 フレームあたり 1 回以下（%d 回 ≤ %d フレーム）");
}

ABH_AimPreview::ABH_AimPreview()
{
	PrimaryActorTick.bCanEverTick = true;
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

void ABH_AimPreview::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// (FCountingAllocator 는 보통 TArray 와 똑같고, 메모리를 잡을 때마다 숫자만 센다. Core/BugHuntProbe.h)
	// [JA] （FCountingAllocator は普通の TArray とまったく同じで、メモリを確保するたびに回数を数えるだけ。Core/BugHuntProbe.h）
	TArray<FVector, BugHunt::FCountingAllocator> Points;
	const float Step = 0.05f;
	FVector Position = GetActorLocation();
	FVector Velocity = ThrowVelocity;
	for (int32 i = 0; i < NumPoints; ++i)
	{
		Points.Add(Position);
		Velocity.Z -= 980.f * Step;
		Position += Velocity * Step;
	}
	LastNumPoints = Points.Num();
	LastLanding = Points.Last();
}

void ABH_Lab54::StartLab()
{
	BugHunt::SetFixedFrameRate(60.f);
	ABH_AimPreview* Preview = SpawnLabActor<ABH_AimPreview>();
	Preview->NumPoints = BugHunt::CaseInt(64, 32, 200);
	BugHunt::Note(BugHunt::Fmt<Lab54Text::Start>(Preview->NumPoints));

	After(0.f, [this, Preview]
	{
		const int32 Start = BugHunt::ArrayReallocCount();
		const uint64 StartFrame = GFrameCounter;
		After(1.0f, [this, Preview, Start, StartFrame]
		{
			const int32 Reallocs = BugHunt::ArrayReallocCount() - Start;
			const int32 Frames = static_cast<int32>(GFrameCounter - StartFrame);
			BugHunt::Note(BugHunt::Fmt<Lab54Text::Allocs>(Frames, Reallocs, Reallocs / FMath::Max(1.f, static_cast<float>(Frames))));
			BugHunt::Expect(BugHunt::Tr<Lab54Text::NumPoints>(), Preview->NumPoints, Preview->LastNumPoints);
			BugHunt::ExpectTrue(BugHunt::Fmt<Lab54Text::PerFrame>(Reallocs, Frames), Reallocs <= Frames);
			FinishLab();
		});
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 결과는 맞는데 메모리를 너무 자주 잡습니다. Points.Add 줄에 조건부 중단점 "Points.Num() == Points.Max()" 를 걸어 보세요.
// 한 프레임에 몇 번 멈추나요? 멈출 때마다 Points.Max() 는 얼마에서 얼마로 늘어나나요?
// 실제 게임에서는 Unreal Insights(-trace=cpu,memalloc) 의 Memory Insights 나 stat memory 로 "프레임마다 할당" 을 찾습니다.

// [JA] 結果は合っていますが、メモリを確保する回数が多すぎます。Points.Add の行に条件付きブレークポイント "Points.Num() == Points.Max()" を設定してみてください。
// [JA] 1 フレームに何回止まりますか？ 止まるたびに Points.Max() はいくつからいくつに増えますか？
// [JA] 実際のゲームでは、Unreal Insights（-trace=cpu,memalloc）の Memory Insights や stat memory で「フレームごとの確保」を探します。
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// TArray 는 공간이 모자랄 때마다 더 큰 메모리를 새로 잡고 옮깁니다. 몇 개가 들어갈지 알면 미리 말해 줄 수 있습니다.
// 또, 지역 배열은 Tick 이 끝날 때마다 메모리를 돌려주고 다음 Tick 에 다시 잡습니다.
// FString::Printf 로 매 프레임 문자열을 만드는 것도 같은 종류의 비용입니다(문자열도 힙 메모리).

// [JA] TArray は容量が足りなくなるたびに、より大きなメモリを新しく確保して移します。いくつ入るか分かっていれば、前もって伝えておけます。
// [JA] また、ローカル配列は Tick が終わるたびにメモリを返し、次の Tick でまた確保します。
// [JA] FString::Printf で毎フレーム文字列を作るのも同じ種類のコストです（文字列もヒープメモリ）。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 들어갈 개수를 먼저 알려 줍니다:
//     Points.Reserve(NumPoints);                 // 프레임당 1번
// 더 줄이려면 배열을 멤버로 두고 Reset() 으로 비웁니다(메모리는 남겨 둠):
//     Points.Reset(); ...                         // 처음 한 번 뒤로 0번
// 개수가 작고 정해져 있으면 TArray<FVector, TInlineAllocator<64>> 처럼 스택에 둘 수도 있습니다.

// [JA] 入る個数を先に伝えます:
// [JA]     Points.Reserve(NumPoints);                 // 1 フレームあたり 1 回
// [JA] さらに減らすには、配列をメンバーにして Reset() で空にします（メモリは残しておく）:
// [JA]     Points.Reset(); ...                         // 最初の 1 回の後は 0 回
// [JA] 個数が少なく決まっているなら、TArray<FVector, TInlineAllocator<64>> のようにスタックに置くこともできます。
#pragma endregion
