#include "Labs/Lab04_QuestLog.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Engine/Engine.h"

void UBH_QuestLog::AddEntry(const FString& Quest)
{
	Entries.Add(Quest);
	UE_LOG(LogBugHunt, Log, TEXT("  퀘스트 기록: %s (총 %d개)"), *Quest, Entries.Num());
}

void ABH_QuestGiver::BeginPlay()
{
	Super::BeginPlay();
	QuestLog = NewObject<UBH_QuestLog>(this, TEXT("QuestLog"));
}

void ABH_QuestGiver::AcceptQuest(const FString& Quest)
{
	QuestLog->AddEntry(Quest);
}

int32 ABH_QuestGiver::NumAccepted() const
{
	return QuestLog->NumEntries();
}

void ABH_Lab04::StartLab()
{
	Giver = SpawnLabActor<ABH_QuestGiver>();
	Giver->AcceptQuest(TEXT("늑대 다섯 마리 사냥"));

	// 실제 게임에서는 GC가 약 60초마다 돈다. 1분을 기다리지 않도록 지금 바로 GC를 돌린다.
	// (콘솔 명령 `obj gc` 와 같다.)
	// GC 전에 퀘스트 기록을 약한 포인터로 지켜본다 (붙잡지는 않는다). GC 뒤 크래시 여부는 메모리 상태에 따라 "가끔"이라,
	// 판정은 이 약한 포인터로 먼저 한다.
	const TWeakObjectPtr<UBH_QuestLog> Watch = FindObject<UBH_QuestLog>(Giver, TEXT("QuestLog"));
	BugHunt::Note(TEXT("... 시간이 흐른다 (GC 실행) ..."));
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
	BugHunt::ExpectTrue(TEXT("GC 뒤에도 퀘스트 기록이 살아 있음"), Watch.IsValid());

	Giver->AcceptQuest(TEXT("대장장이에게 광석 전달"));
	BugHunt::Expect(TEXT("받은 퀘스트 수"), 2, Giver->NumAccepted());
	FinishLab();
}

// ============================================================================
	// [JA] 実際のゲームでは GC は約 60 秒ごとに走る。1 分待たなくて済むよう、今すぐ GC を走らせる。
	// [JA] （コンソール コマンド `obj gc` と同じ。）
	// [JA] GC の前にクエスト記録を弱いポインターで見張る（つかんではおかない）。GC 後にクラッシュするかはメモリの状態しだいで「ときどき」なので、
	// [JA] 判定はまずこの弱いポインターで行う。
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 이 랩은 "해제된 메모리 사용"이라 증상이 매번 다를 수 있습니다:
// 읽기/쓰기/실행 액세스 위반, 호출 스택 맨 위에 이상한 주소(0x00000000xxxxxxxx), 또는 조용히 틀린 숫자.
// 엉뚱한 곳에서 터지면 호출 스택에서 ABH_ 로 시작하는 첫 줄로 가세요. AcceptQuest 의 QuestLog 를
// 지역 창에서 펼치면 Entries 나 이름(NamePrivate)이 정상인가요?
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 같은 객체를 GC 전과 후에 비교하세요. StartLab 의 CollectGarbage 줄에 중단점 → 조사식에
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
//     Giver->QuestLog
// 를 넣고 주소와 내용을 적어 둡니다. F10 으로 GC를 넘긴 뒤 다시 보세요.
// 실제 게임에서 재현하려면 콘솔에 gc.CollectGarbageEveryFrame 1 을 넣으면 매 프레임 GC가 돌아
// 이런 버그가 바로 드러납니다.
#pragma endregion

#pragma region 힌트 3 / ヒント 3

// [JA] このラボは「解放されたメモリの使用」なので、症状が毎回違うことがあります:
// [JA] 読み取り／書き込み／実行のアクセス違反、呼び出し履歴のいちばん上におかしなアドレス（0x00000000xxxxxxxx）、または黙って間違った数字。
// [JA] 見当違いの場所で落ちたら、呼び出し履歴で ABH_ で始まる最初の行へ行ってください。AcceptQuest の QuestLog を
// [JA] ローカル ウィンドウで展開すると、Entries や名前（NamePrivate）は正常ですか？
// GC는 "UPROPERTY 로 이어진 참조"만 따라가서 살아 있는 객체를 찾습니다.
// Outer(NewObject 의 첫 인자)는 객체를 살려 두지 않습니다. 헤더에서 QuestLog 선언을 다시 보세요.
// 증상을 확실하게 하려면 명령줄에 -stomp2malloc 을 붙여 실행하세요. 해제된 메모리를 건드리는 순간 바로 멈춥니다.
#pragma endregion

// [JA] 同じオブジェクトを GC の前と後で比べてください。StartLab の CollectGarbage の行にブレークポイント → ウォッチに
// [JA]     Giver->QuestLog
// [JA] を入れて、アドレスと内容を書き留めておきます。F10 で GC を越えたあと、もう一度見てください。
// [JA] 実際のゲームで再現するには、コンソールに gc.CollectGarbageEveryFrame 1 を入れると毎フレーム GC が走り、
// [JA] この種のバグがすぐに表に出ます。

// [JA] GC は「UPROPERTY でつながった参照」だけをたどって、生きているオブジェクトを探します。
// [JA] Outer（NewObject の最初の引数）はオブジェクトを生かしておきません。ヘッダーで QuestLog の宣言をもう一度見てください。
// [JA] 症状をはっきりさせるには、コマンドラインに -stomp2malloc を付けて実行してください。解放されたメモリに触れた瞬間に止まります。
