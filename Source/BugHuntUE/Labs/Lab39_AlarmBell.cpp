// [주의] 이 랩은 소리를 냅니다. 실행 전에 시스템 볼륨을 낮추고, 헤드폰을 쓴다면 귀에서 떼고 시작하세요.
//        (실습용 소리는 볼륨 배율 0.5 로 재생합니다.)
// [JA] [注意] このラボは音が出ます。実行前にシステムの音量を下げ、ヘッドホンを使っている場合は耳から離してから始めてください。
// [JA]        （実習用の音は音量倍率 0.5 で再生します。）

#include "Labs/Lab39_AlarmBell.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "UObject/ConstructorHelpers.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab39Text
{
BH_TEXT(Ding,       "  %s 땡! (%d번째)", "  %s カーン！（%d 回目）");
BH_TEXT(HeardCount, "%s 들은 종소리 횟수", "%s が聞いた鐘の音の回数");
BH_TEXT(Caution,    "[주의] 소리가 납니다. 볼륨을 낮추세요.", "[注意] 音が出ます。音量を下げてください。");
BH_TEXT(SkipName,   "랩 39", "ラボ 39");
BH_TEXT(RunOnServer, "서버(리슨 서버 창)에서 실행하세요.", "サーバー（リッスンサーバーのウィンドウ）で実行してください。");
BH_TEXT(Standalone, "스탠드얼론에서는 통과합니다. 리슨 서버 + 클라이언트로 실행하세요 (README 10부).", "スタンドアロンでは通ります。リッスンサーバー + クライアントで実行してください（README 第 10 部）。");
BH_TEXT(Scenario,   "적이 세 번 성문에 닿는다 → 종이 세 번 울려야 한다.", "敵が 3 回城門に着く → 鐘が 3 回鳴らなければならない。");
}

ABH_AlarmBell::ABH_AlarmBell()
{
	bReplicates = true;
	bAlwaysRelevant = true;

	static ConstructorHelpers::FObjectFinder<USoundBase> Ping(TEXT("/Engine/EngineSounds/1kSineTonePing.1kSineTonePing"));
	RingSound = Ping.Object;
}

void ABH_AlarmBell::Ring()
{
	check(HasAuthority());
	PlayRingSound();
}

void ABH_AlarmBell::PlayRingSound()
{
	UGameplayStatics::PlaySoundAtLocation(this, RingSound, GetActorLocation(), /*VolumeMultiplier*/ 0.5f);
	++RingsHeard;
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab39Text::Ding>(*BugHunt::NetTag(this), RingsHeard));
}

void ABH_AlarmBell::Multicast_CheckHeard_Implementation(int32 ExpectedRings)
{
	BugHunt::Expect(BugHunt::Fmt<Lab39Text::HeardCount>(*BugHunt::NetTag(this)), ExpectedRings, RingsHeard);
	if (GetNetMode() == NM_Client)
	{
		BugHunt::ExitIfAutomated();
	}
}

// ---------------------------------------------------------------------------
// 아래는 서버에서 도는 시나리오입니다.
// [JA] 以下はサーバーで動くシナリオです。

void ABH_Lab39::StartLab()
{
	BugHunt::Note(BugHunt::Tr<Lab39Text::Caution>());
	if (GetNetMode() == NM_Client)
	{
		BugHunt::Skip(BugHunt::Tr<Lab39Text::SkipName>(), BugHunt::Tr<Lab39Text::RunOnServer>());
		FinishLab();
		return;
	}
	if (GetNetMode() == NM_Standalone)
	{
		BugHunt::Note(BugHunt::Tr<Lab39Text::Standalone>());
	}

	WaitForClientThen([this]
	{
		ABH_AlarmBell* Bell = SpawnLabActor<ABH_AlarmBell>(FVector(0.f, 500.f, 200.f));
		BugHunt::Note(BugHunt::Tr<Lab39Text::Scenario>());

		// 종이 클라이언트에 복제될 시간을 준 뒤 친다.
		// [JA] 鐘がクライアントにレプリケーションされる時間を与えてから鳴らす。
		After(0.5f, [Bell] { Bell->Ring(); });
		After(1.0f, [Bell] { Bell->Ring(); });
		After(1.5f, [Bell] { Bell->Ring(); });

		After(2.5f, [Bell] { Bell->Multicast_CheckHeard(3); });
		After(3.5f, [this] { FinishLab(); });
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// PIE를 Number of Players 2, Play As Listen Server 로 실행하고 서버 창에서 BugHunt.Lab 39.
// 귀로 확인하기는 어렵습니다(PIE 창들이 한 스피커를 같이 쓰고, 포커스 없는 창은 소리가 줄어들 수 있습니다). 로그로 확인하세요.
// Output Log에서 "땡!" 줄 앞의 꼬리표가 [리슨 서버]뿐인지 보세요.

// [JA] PIE を Number of Players 2、Play As Listen Server で実行し、サーバーのウィンドウで BugHunt.Lab 39。
// [JA] 耳で確かめるのは難しいです（PIE ウィンドウどうしが 1 つのスピーカーを共有し、フォーカスのないウィンドウは音が小さくなることがあります）。ログで確認してください。
// [JA] アウトプットログで「カーン！」の行の前のタグが [リッスンサーバー] だけかどうか見てください。
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// PlayRingSound 에 중단점 → 멈출 때마다 조사식에 GetNetMode(). 클라이언트에서 한 번이라도 멈추나요?
// UGameplayStatics::PlaySoundAtLocation 은 "그 함수를 부른 머신"의 스피커에서만 소리를 냅니다. 소리는 복제되지 않습니다.

// [JA] PlayRingSound にブレークポイント → 止まるたびにウォッチに GetNetMode()。クライアントで一度でも止まりますか？
// [JA] UGameplayStatics::PlaySoundAtLocation は「その関数を呼んだマシン」のスピーカーからしか音を出しません。音はレプリケーションされません。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 서버에서 일어난 일을 모든 머신에서 "연출"(소리, 파티클)하려면 NetMulticast RPC 로 각 머신에서 재생하게 합니다.
//     UFUNCTION(NetMulticast, Unreliable) void Multicast_PlayRing();
// (연출은 하나쯤 빠져도 되므로 보통 Unreliable. 서버에서 부르면 서버 자신에서도 실행됩니다.)
// 데디케이티드 서버라면 지금 코드로는 아무도 못 듣습니다. 서버에는 스피커가 없으니까요.

// [JA] サーバーで起きたことをすべてのマシンで「演出」（音、パーティクル）するには、NetMulticast RPC で各マシンで再生させます。
// [JA]     UFUNCTION(NetMulticast, Unreliable) void Multicast_PlayRing();
// [JA] （演出は 1 つくらい抜けても構わないので普通は Unreliable。サーバーで呼ぶとサーバー自身でも実行されます。）
// [JA] デディケイテッドサーバーなら、今のコードでは誰も聞こえません。サーバーにはスピーカーがないからです。
#pragma endregion
