#include "Labs/Lab40_Elevator.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntHang.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab40Text
{
BH_TEXT(ArrivedLog,   "  %.2f초: 엘리베이터 도착 (높이 %.0f, Tick %d번)", "  %.2f 秒: エレベーター到着（高さ %.0f、Tick %d 回）");
BH_TEXT(WaitElevator, "엘리베이터 기다리기", "エレベーター待ち");
BH_TEXT(BoardLog,     "  %.2f초: 승객 탑승 (엘리베이터 높이 %.0f)", "  %.2f 秒: 乗客が搭乗（エレベーターの高さ %.0f）");
BH_TEXT(Plan,         "승객이 높이 %.0f 에서 엘리베이터를 부른다. 속도 %.0f 이면 %.2f초 뒤 도착.", "乗客が高さ %.0f でエレベーターを呼ぶ。速度 %.0f なら %.2f 秒後に到着。");
BH_TEXT(Boarded,      "승객이 탔다", "乗客が乗った");
BH_TEXT(WasThere,     "탈 때 엘리베이터가 와 있었다", "乗るときにエレベーターが来ていた");
BH_TEXT(Blocked,      "엘리베이터를 부르는 동안 게임이 멈춘 시간 < 100 ms (실제: %.0f ms)", "エレベーターを呼んでいる間にゲームが止まった時間 < 100 ms（実際: %.0f ms）");
BH_TEXT(Arrived,      "엘리베이터 도착", "エレベーター到着");
}

ABH_Elevator::ABH_Elevator()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
}

void ABH_Elevator::Call(float InTargetHeight, float InSpeed)
{
	TargetHeight = InTargetHeight;
	Speed = InSpeed;
	bArrived = false;
	SetActorTickEnabled(true);
}

void ABH_Elevator::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	++TicksWhileMoving;
	Height = FMath::Min(TargetHeight, Height + Speed * DeltaSeconds);
	if (Height >= TargetHeight)
	{
		bArrived = true;
		SetActorTickEnabled(false);
		UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab40Text::ArrivedLog>(GetWorld()->GetTimeSeconds(), Height, TicksWhileMoving));
		OnArrived.Broadcast();
	}
}

void ABH_Passenger::RideElevator(ABH_Elevator* Elevator, float TargetHeight, float Speed)
{
	Elevator->Call(TargetHeight, Speed);

	// 엘리베이터가 올 때까지 기다렸다가 탄다.
	// [JA] エレベーターが来るまで待ってから乗る。
	BugHunt::FHangGuard Guard(BugHunt::Tr<Lab40Text::WaitElevator>());
	while (!Elevator->bArrived)
	{
		BH_HANG_CHECK(Guard);   // 실습용 안전장치 (Core/BugHuntHang.h). 버그는 이 줄이 아니다.
		// [JA] 実習用の安全装置（Core/BugHuntHang.h）。バグはこの行ではない。
		FPlatformProcess::Sleep(0.01f);   // CPU 를 덜 쓰게 잠깐 쉰다
		// [JA] CPU を使いすぎないように少し休む
	}
	Board(Elevator);
}

void ABH_Passenger::Board(ABH_Elevator* Elevator)
{
	bBoarded = true;
	bElevatorWasThere = Elevator->bArrived;
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab40Text::BoardLog>(GetWorld()->GetTimeSeconds(), Elevator->Height));
}

void ABH_Lab40::StartLab()
{
	const float TargetHeight = BugHunt::CaseFloat(300.f, 200.f, 600.f);
	const float Speed = BugHunt::CaseFloat(600.f, 400.f, 1000.f);

	ABH_Elevator* Elevator = SpawnLabActor<ABH_Elevator>();
	ABH_Passenger* Passenger = SpawnLabActor<ABH_Passenger>(FVector(0.f, 0.f, TargetHeight));
	BugHunt::Note(BugHunt::Fmt<Lab40Text::Plan>(
		TargetHeight, Speed, TargetHeight / Speed));

	const double CallStart = FPlatformTime::Seconds();
	Passenger->RideElevator(Elevator, TargetHeight, Speed);
	const double BlockedMs = (FPlatformTime::Seconds() - CallStart) * 1000.0;

	After(TargetHeight / Speed + 1.0f, [this, Elevator, Passenger, BlockedMs]
	{
		BugHunt::ExpectTrue(BugHunt::Tr<Lab40Text::Boarded>(), Passenger->bBoarded);
		BugHunt::ExpectTrue(BugHunt::Tr<Lab40Text::WasThere>(), Passenger->bElevatorWasThere);
		BugHunt::ExpectTrue(BugHunt::Fmt<Lab40Text::Blocked>(BlockedMs),
			BlockedMs < 100.0);
		BugHunt::ExpectTrue(BugHunt::Tr<Lab40Text::Arrived>(), Elevator->bArrived);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 진짜 게임이라면 여기서 에디터가 굳습니다. 그 상황을 직접 겪어 보세요:
//   1) 콘솔에 BugHunt.HangBudgetSeconds 30 을 넣고 BugHunt.Lab 40 → 게임 화면이 30초 동안 굳습니다.
//   2) 굳어 있는 동안 VS 에서 디버그 → 모두 중단 (Ctrl+Alt+Break).
//   3) 디버그 → 창 → 스레드 (Ctrl+Alt+H). "GameThread" 를 더블클릭하고 호출 스택을 보세요. 어디서 돌고 있나요?
//   4) 조사식에 Elevator->Height, Elevator->TicksWhileMoving 을 넣고 F5 → 다시 모두 중단. 값이 변하나요?

// [JA] 本物のゲームなら、ここでエディターが固まります。その状況を実際に体験してみてください:
// [JA]   1) コンソールに BugHunt.HangBudgetSeconds 30 を入れて BugHunt.Lab 40 → ゲーム画面が 30 秒間固まります。
// [JA]   2) 固まっている間に VS で デバッグ → すべて中断（Ctrl+Alt+Break）。
// [JA]   3) デバッグ → ウィンドウ → スレッド（Ctrl+Alt+H）。"GameThread" をダブルクリックして呼び出し履歴を見てください。どこで回っていますか？
// [JA]   4) ウォッチに Elevator->Height, Elevator->TicksWhileMoving を入れて F5 → もう一度すべて中断。値は変わりますか？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 엘리베이터를 움직이는 것은 Tick 입니다. Tick 은 누가, 어느 스레드에서 부르나요?
// 게임 스레드 하나가 "Tick 돌리기 → 그리기 → 다음 프레임"을 차례로 합니다. 그 게임 스레드가 while 안에서 기다리면
// 다음 프레임이 오지 않고, Tick 도 오지 않고, bArrived 는 영원히 false 입니다. Sleep 을 넣어도 마찬가지입니다.
// 게임 코드에서 "다른 객체가 바꿔 줄 값을 while 로 기다리기"는 거의 항상 이 버그입니다.

// [JA] エレベーターを動かしているのは Tick です。Tick は誰が、どのスレッドで呼ぶのでしょう？
// [JA] 1 つのゲームスレッドが「Tick を回す → 描画 → 次のフレーム」を順番に行います。そのゲームスレッドが while の中で待つと、
// [JA] 次のフレームが来ず、Tick も来ず、bArrived は永遠に false です。Sleep を入れても同じです。
// [JA] ゲームコードで「別のオブジェクトが変えてくれる値を while で待つ」のは、ほぼ必ずこのバグです。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 기다리지 말고 "도착하면 불러 달라"고 등록하세요. 엘리베이터에는 이미 OnArrived 델리게이트가 있습니다.
//     Elevator->Call(TargetHeight, Speed);
//     Elevator->OnArrived.AddUObject(this, &ABH_Passenger::Board, Elevator);
// (Board 에 인자를 더 넘길 수 있습니다: AddUObject 의 뒤쪽 인자는 페이로드.)
// 매 프레임 확인하는 방법도 됩니다: 타이머(SetTimer 0.05초 반복)나 이 액터의 Tick 에서 bArrived 를 보고 탑승.
// 핵심은 게임 스레드를 돌려주는 것입니다.

// [JA] 待つのではなく、「到着したら呼んでほしい」と登録してください。エレベーターにはすでに OnArrived デリゲートがあります。
// [JA]     Elevator->Call(TargetHeight, Speed);
// [JA]     Elevator->OnArrived.AddUObject(this, &ABH_Passenger::Board, Elevator);
// [JA] （Board に引数を追加で渡せます: AddUObject の後ろの引数はペイロード。）
// [JA] 毎フレーム確認する方法でもかまいません: タイマー（SetTimer 0.05 秒の繰り返し）や、このアクターの Tick で bArrived を見て搭乗。
// [JA] 肝心なのは、ゲームスレッドを返してあげることです。
#pragma endregion
