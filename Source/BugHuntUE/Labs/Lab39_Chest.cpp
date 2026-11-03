#include "Labs/Lab39_Chest.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"

ABH_Chest::ABH_Chest()
{
	bReplicates = true;
	bAlwaysRelevant = true;
}

void ABH_Chest::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABH_Chest, bOpened);
}

void ABH_Chest::Open(ABH_PlayerController* Opener)
{
	if (bOpened)
	{
		return;
	}
	bOpened = true;
	Opener->Gold += Reward;
	UE_LOG(LogBugHunt, Log, TEXT("  %s 상자 열림! %s 골드 %d"), *BugHunt::NetTag(this), *Opener->GetName(), Opener->Gold);
}

void ABH_PlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ABH_PlayerController, Gold, COND_OwnerOnly);
}

void ABH_PlayerController::InteractWithChest(ABH_Chest* Chest)
{
	if (Chest)
	{
		Chest->Open(this);
	}
}

void ABH_PlayerController::Client_PressInteract_Implementation(ABH_Chest* Chest)
{
	BugHunt::Note(FString::Printf(TEXT("%s E 키 입력"), *BugHunt::NetTag(this)));
	InteractWithChest(Chest);

	// 1초 뒤 이 플레이어 화면에 보이는 골드를 적는다.
	FTimerHandle NoteTimer;
	GetWorldTimerManager().SetTimer(NoteTimer, FTimerDelegate::CreateWeakLambda(this, [this]
	{
		BugHunt::Note(FString::Printf(TEXT("%s 내 화면의 골드: %d"), *BugHunt::NetTag(this), Gold));
	}), 1.0f, false);

	// 자동 검증용: 서버가 검사를 끝낼 때까지 접속을 유지한 뒤 종료한다.
	FTimerHandle ExitTimer;
	GetWorldTimerManager().SetTimer(ExitTimer, FTimerDelegate::CreateWeakLambda(this, []
	{
		BugHunt::ExitIfAutomated();
	}), 3.0f, false);
}

// ---------------------------------------------------------------------------
// 아래는 서버에서 도는 시나리오입니다.

	// [JA] 1 秒後、このプレイヤーの画面に見えているゴールドを記録する。
void ABH_Lab39::StartLab()
{
	if (GetNetMode() == NM_Client)
	{
		BugHunt::Skip(TEXT("랩 39"), TEXT("서버(리슨 서버 창)에서 실행하세요."));
		FinishLab();
		return;
	// [JA] 自動検証用: サーバーが検査を終えるまで接続を保ってから終了する。
	}
	WaitForClientThen([this] { RunScenario(); });
}

void ABH_Lab39::RunScenario()
{
	TArray<ABH_PlayerController*> Players;
	TArray<ABH_Chest*> Chests;
	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
// [JA] 以下はサーバーで動くシナリオです。
	{
		if (ABH_PlayerController* PC = Cast<ABH_PlayerController>(It->Get()))
		{
			Players.Add(PC);
			Chests.Add(SpawnLabActor<ABH_Chest>(FVector(200.f * Players.Num(), 300.f, 50.f)));
		}
	}
	BugHunt::Note(FString::Printf(TEXT("플레이어 %d명. 각자 자기 앞의 상자를 연다."), Players.Num()));

	// 상자가 클라이언트에 복제될 시간을 준 뒤 입력을 재생한다.
	After(0.5f, [this, Players, Chests]
	{
		for (int32 i = 0; i < Players.Num(); ++i)
		{
			if (Players[i]->IsLocalController())
			{
				Players[i]->InteractWithChest(Chests[i]);     // 호스트(이 머신의 플레이어)
			}
			else
			{
				Players[i]->Client_PressInteract(Chests[i]);  // 원격 플레이어의 머신에서 E 키
			}
		}
	});

	After(2.0f, [this, Players, Chests]
	{
	// [JA] 宝箱がクライアントにレプリケーションされる時間を与えてから、入力を再生する。
		int32 RemoteCount = 0;
		for (int32 i = 0; i < Players.Num(); ++i)
		{
			const bool bLocal = Players[i]->IsLocalController();
			RemoteCount += bLocal ? 0 : 1;
			const FString Who = bLocal ? TEXT("호스트") : FString::Printf(TEXT("원격 플레이어 %d"), RemoteCount);
			BugHunt::ExpectTrue(FString::Printf(TEXT("[서버 기준] %s의 상자 열림"), *Who), Chests[i]->bOpened);
				// [JA] ホスト（このマシンのプレイヤー）
			BugHunt::Expect(FString::Printf(TEXT("[서버 기준] %s의 골드"), *Who), ABH_Chest::Reward, Players[i]->Gold);
		}
		if (RemoteCount == 0)
		{
				// [JA] リモートプレイヤーのマシンで E キー
			BugHunt::Skip(TEXT("원격 플레이어 검사"),
				TEXT("접속한 클라이언트가 없습니다. PIE를 Number of Players 2, Play As Listen Server 로 실행하세요."));
		}
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 스탠드얼론으로 돌리면 통과합니다. 버그는 "원격 플레이어"에게만 생깁니다.
// PIE를 Number of Players 2, Net Mode "Play As Listen Server" 로 실행하고, 서버 창에서 BugHunt.Lab 39.
// 클라이언트 로그의 "내 화면의 골드" 와 서버의 [FAIL] 줄을 비교하세요. 누구 말이 맞나요?
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// ABH_Chest::Open 에 중단점을 걸고 멈출 때마다 조사식에 GetNetMode() 와 HasAuthority() 를 넣어 보세요.
// 원격 플레이어의 상자를 연 Open 은 어느 머신(서버/클라이언트)에서 실행됐나요?
// 클라이언트에서 복제 변수(bOpened, Gold)를 바꾸면 그 값은 서버로 갈까요?
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 복제는 서버 → 클라이언트 한 방향뿐입니다. 클라이언트가 서버에게 부탁하려면 Server RPC 가 필요합니다:
//     UFUNCTION(Server, Reliable) void Server_OpenChest(ABH_Chest* Chest);
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// (Server RPC 는 그 클라이언트가 "소유한" 액터, 예를 들어 자기 PlayerController 에서 불러야 서버에 도착합니다.)
// 헤더에 UFUNCTION 을 추가하는 변경은 Live Coding 으로 적용되지 않습니다. 에디터를 닫고 빌드하세요.
#pragma endregion

// [JA] スタンドアロンで動かすと合格します。バグは「リモートプレイヤー」にだけ起きます。
// [JA] PIE を Number of Players 2、Net Mode "Play As Listen Server" で実行し、サーバーのウィンドウで BugHunt.Lab 39。
// [JA] クライアントのログの「自分の画面のゴールド」とサーバーの [FAIL] の行を比べてください。どちらの言い分が正しいですか？

// [JA] ABH_Chest::Open にブレークポイントを置き、止まるたびにウォッチに GetNetMode() と HasAuthority() を入れてみてください。
// [JA] リモートプレイヤーの宝箱を開けた Open は、どのマシン（サーバー／クライアント）で実行されましたか？
// [JA] クライアントでレプリケーション変数（bOpened, Gold）を変えると、その値はサーバーへ届くでしょうか？

// [JA] レプリケーションはサーバー → クライアントの一方向だけです。クライアントがサーバーに頼むには Server RPC が必要です:
// [JA]     UFUNCTION(Server, Reliable) void Server_OpenChest(ABH_Chest* Chest);
// [JA] （Server RPC は、そのクライアントが「所有する」アクター、たとえば自分の PlayerController から呼ばないとサーバーに届きません。）
// [JA] ヘッダーに UFUNCTION を追加する変更は Live Coding では適用されません。エディターを閉じてビルドしてください。
