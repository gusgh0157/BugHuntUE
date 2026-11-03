#include "Labs/Lab13_RoundStats.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

void ABH_RoundBoard::StartRound(const TArray<FString>& Players)
{
	++Round;
	Stats.Reset();      // 지난 라운드 기록을 비운다 (메모리는 다시 쓰려고 남겨 둔다)
	for (const FString& Player : Players)
	{
		FBH_RoundStats& Entry = Stats.AddDefaulted_GetRef();
		Entry.Player = Player;
	}
}

void ABH_RoundBoard::RecordKill(int32 Killer, int32 Victim)
{
	// [JA] 前のラウンドの記録を空にする（メモリは再利用のために残しておく）
	++Stats[Killer].Kills;
	++Stats[Victim].Deaths;
}

void ABH_Lab13::StartLab()
{
	ABH_RoundBoard* Board = SpawnLabActor<ABH_RoundBoard>();
	const TArray<FString> Players = { TEXT("Aria"), TEXT("Bram"), TEXT("Cole") };

	// 공개 입력: 1라운드 6킬, 2라운드 3킬
	const int32 Round1Kills = BugHunt::CaseInt(6, 3, 12);
	const int32 Round2Kills = BugHunt::CaseInt(3, 1, 12);

	Board->StartRound(Players);
	for (int32 i = 0; i < Round1Kills; ++i)
	{
		Board->RecordKill(i % 3, (i + 1) % 3);
	}
	BugHunt::Note(FString::Printf(TEXT("%d라운드 끝: 킬 %d번"), Board->Round, Round1Kills));
	// [JA] 公開入力: ラウンド 1 は 6 キル、ラウンド 2 は 3 キル

	Board->StartRound(Players);
	TArray<int32> Expected = { 0, 0, 0 };
	for (int32 i = 0; i < Round2Kills; ++i)
	{
		const int32 Killer = (i * 2) % 3;
		Board->RecordKill(Killer, (Killer + 2) % 3);
		++Expected[Killer];
	}
	BugHunt::Note(FString::Printf(TEXT("%d라운드 끝: 킬 %d번"), Board->Round, Round2Kills));

	BugHunt::Expect(TEXT("지금 라운드 번호"), 2, Board->Round);
	for (int32 p = 0; p < Players.Num(); ++p)
	{
		BugHunt::Expect(FString::Printf(TEXT("2라운드 %s 킬"), *Players[p]), Expected[p], Board->Stats[p].Kills);
	}
	int32 TotalDeaths = 0;
	for (const FBH_RoundStats& Entry : Board->Stats)
	{
		TotalDeaths += Entry.Deaths;
	}
	BugHunt::Expect(TEXT("2라운드 전체 데스"), Round2Kills, TotalDeaths);
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 2라운드 기록에 1라운드 숫자가 섞였습니다. 두 번째 StartRound 의 AddDefaulted_GetRef 줄 다음에 중단점을 걸고 Entry 를 보세요.
// Player 는 새 이름인데 Kills 와 Deaths 는요? 에디터를 켤 때 Output Log 에 찍히는 "is not initialized properly" 빨간 줄도 찾아 보세요.
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// UObject(액터 포함)는 메모리가 0 으로 채워진 뒤 생성자가 불립니다. 그래서 ABH_RoundBoard::Round 는 초기값 없이도 0 에서 시작했습니다.
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// 일반 구조체(USTRUCT 포함), 지역 변수, FMemory::Malloc 으로 받은 메모리는 아무도 0 으로 채우지 않습니다. 생성자가 비어 있으면 그 자리에 있던 값이 그대로입니다.
// 여기서 그 자리는 Stats.Reset() 이 비우고 남겨 둔 지난 라운드의 메모리입니다. Debug 빌드나 다른 PC 에서는 다른 쓰레기 값이 나올 수 있습니다.
#pragma endregion

#pragma region 힌트 3 / ヒント 3

// [JA] ラウンド 2 の記録にラウンド 1 の数字が混ざりました。2 回目の StartRound の AddDefaulted_GetRef の行の次にブレークポイントを設定し、Entry を見てください。
// [JA] Player は新しい名前ですが、Kills と Deaths は？ エディターを起動したときにアウトプットログに出る "is not initialized properly" の赤い行も探してみてください。
// 멤버마다 초기값을 적습니다 (UE 코딩 규칙도 그렇습니다):
//     UPROPERTY() int32 Kills = 0;
//     UPROPERTY() int32 Deaths = 0;
// AddZeroed 로 바꾸는 것은 이 호출 한 곳만 고칩니다. 지역 변수로 FBH_RoundStats S; 를 만드는 다른 곳은 여전히 쓰레기 값입니다.
// UObject 멤버(Round)도 0 이 아닌 값에서 시작해야 하면 초기값을 적어야 합니다. 0 이 우연이 아니라 의도라는 표시도 됩니다.
#pragma endregion

// [JA] UObject（アクターを含む）はメモリが 0 で埋められた後にコンストラクタが呼ばれます。そのため ABH_RoundBoard::Round は初期値なしでも 0 から始まりました。
// [JA] 普通の構造体（USTRUCT を含む）、ローカル変数、FMemory::Malloc で得たメモリは誰も 0 で埋めません。コンストラクタが空なら、その場所にあった値がそのまま残ります。
// [JA] ここでのその場所は、Stats.Reset() が空にして残しておいた前のラウンドのメモリです。Debug ビルドや別の PC では別のゴミ値が出ることがあります。

// [JA] メンバーごとに初期値を書きます（UE のコーディング規約もそうなっています）:
// [JA]     UPROPERTY() int32 Kills = 0;
// [JA]     UPROPERTY() int32 Deaths = 0;
// [JA] AddZeroed に変えるのは、この呼び出し 1 か所だけを直すことになります。ローカル変数で FBH_RoundStats S; を作る別の場所は相変わらずゴミ値です。
// [JA] UObject のメンバー（Round）も 0 以外の値から始める必要があるなら初期値を書かなければなりません。0 が偶然ではなく意図だという印にもなります。
