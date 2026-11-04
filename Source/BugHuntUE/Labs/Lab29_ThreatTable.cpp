#include "Labs/Lab29_ThreatTable.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab29Text
{
BH_TEXT(HitLog,     "  %s -> 보스 %d 피해 (보스 HP %d)", "  %s -> ボスに %d ダメージ（ボス HP %d）");
BH_TEXT(KaelLeft,   "Kael 이 쓰러져 파티를 떠났다. ... 시간이 흐른다 (GC 실행) ...", "Kael が倒れてパーティーを離れた。... 時間が流れる（GC 実行）...");
BH_TEXT(BramJoined, "새 영웅 Bram 합류.", "新しい英雄 Bram が加入。");
BH_TEXT(Unknown,    "알 수 없는 대상", "不明な対象");
BH_TEXT(None,       "없음", "なし");
BH_TEXT(BramThreat, "Bram 의 어그로 (Bram 이 준 피해)", "Bram のヘイト（Bram が与えたダメージ）");
BH_TEXT(LiaThreat,  "Lia 의 어그로", "Lia のヘイト");
BH_TEXT(TopIsLia,   "보스가 노리는 대상 == Lia (실제: %s)", "ボスが狙う対象 == Lia（実際: %s）");
}

void FBH_ThreatTable::AddThreat(AActor* Source, int32 Amount)
{
	Threat.FindOrAdd(Source) += Amount;
}

int32 FBH_ThreatTable::ThreatOf(AActor* Source) const
{
	const int32* Found = Threat.Find(Source);
	return Found ? *Found : 0;
}

AActor* FBH_ThreatTable::TopTarget() const
{
	AActor* Best = nullptr;
	int32 BestThreat = -1;
	for (const TPair<AActor*, int32>& Pair : Threat)
	{
		if (Pair.Key && Pair.Value > BestThreat)
		{
			Best = Pair.Key;
			BestThreat = Pair.Value;
		}
	}
	return Best;
}

void ABH_RaidBoss::TakeHit(ABH_Raider* Attacker, int32 Damage)
{
	Health -= Damage;
	ThreatTable.AddThreat(Attacker, Damage);
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab29Text::HitLog>(*Attacker->DisplayName, Damage, Health));
}

void ABH_Lab29::StartLab()
{
	const int32 KaelDamage = BugHunt::CaseInt(50, 40, 90);
	const int32 LiaDamage = BugHunt::CaseInt(30, 15, KaelDamage - 10);
	const int32 BramDamage = BugHunt::CaseInt(10, 5, LiaDamage - 5);

	ABH_RaidBoss* Boss = SpawnLabActor<ABH_RaidBoss>(FVector(500.f, 0.f, 100.f));
	ABH_Raider* Kael = SpawnLabActor<ABH_Raider>(FVector(0.f, -100.f, 100.f));
	ABH_Raider* Lia = SpawnLabActor<ABH_Raider>(FVector(0.f, 100.f, 100.f));
	Kael->DisplayName = TEXT("Kael");
	Lia->DisplayName = TEXT("Lia");

	Boss->TakeHit(Kael, KaelDamage);
	Boss->TakeHit(Lia, LiaDamage);

	BugHunt::Note(BugHunt::Tr<Lab29Text::KaelLeft>());
	Kael->Destroy();
	Kael = nullptr;
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);

	ABH_Raider* Bram = SpawnLabActor<ABH_Raider>(FVector(0.f, -100.f, 100.f));
	Bram->DisplayName = TEXT("Bram");
	BugHunt::Note(BugHunt::Tr<Lab29Text::BramJoined>());
	Boss->TakeHit(Bram, BramDamage);

	// 보스가 노리는 대상. (표에 남은 포인터는 이미 지워진 영웅일 수도 있으니 역참조하지 않고 비교만 한다.)
	// [JA] ボスが狙う対象。（表に残ったポインターはすでに消えた英雄かもしれないので、逆参照せず比較だけする。）
	AActor* Top = Boss->ThreatTable.TopTarget();
	const FString TopName = Top == Lia ? TEXT("Lia") : Top == Bram ? TEXT("Bram") : Top ? BugHunt::Tr<Lab29Text::Unknown>() : BugHunt::Tr<Lab29Text::None>();

	BugHunt::Expect(BugHunt::Tr<Lab29Text::BramThreat>(), BramDamage, Boss->ThreatTable.ThreatOf(Bram));
	BugHunt::Expect(BugHunt::Tr<Lab29Text::LiaThreat>(), LiaDamage, Boss->ThreatTable.ThreatOf(Lia));
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab29Text::TopIsLia>(*TopName), Top == Lia);
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// StartLab 의 Kael->Destroy() 줄에 중단점을 걸고, 조사식에 Kael 을 넣어 주소를 적어 두세요.
// 그다음 Bram 을 스폰한 줄 뒤에서 Bram 의 주소를 보세요. 그리고 Boss->ThreatTable.Threat 를 펼쳐 키(주소)들을 보세요.
// 표에는 항목이 몇 개 있고, 각 키는 누구인가요?

// [JA] StartLab の Kael->Destroy() の行にブレークポイントを置き、ウォッチに Kael を入れてアドレスを書き留めてください。
// [JA] 次に Bram をスポーンした行の後で Bram のアドレスを見てください。そして Boss->ThreatTable.Threat を展開してキー（アドレス）を見てください。
// [JA] 表には項目がいくつあり、それぞれのキーは誰ですか？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// GC 가 Kael 을 지우면 그 메모리는 할당기로 돌아갑니다. 같은 크기의 객체(Bram)를 바로 만들면 그 자리를
// 다시 쓰는 일이 흔합니다. 그러면 Bram 의 주소 == Kael 이 쓰던 주소. 표는 Bram 을 Kael 로 착각합니다.
// 주소가 달라도 문제입니다: 표에는 지워진 메모리를 가리키는 키가 남아 TopTarget 이 그걸 고릅니다.
// raw 포인터는 "그 주소"를 기억할 뿐, "그 객체가 아직 그 객체인지"는 모릅니다.
// UObject 가 아닌 곳에서 UObject 를 기억해야 할 때 무엇을 써야 할까요? (UPROPERTY 는 못 씁니다)

// [JA] GC が Kael を消すと、そのメモリはアロケーターに戻ります。同じサイズのオブジェクト（Bram）をすぐに作ると、その場所を
// [JA] 再利用することはよくあります。すると Bram のアドレス == Kael が使っていたアドレス。表は Bram を Kael と取り違えます。
// [JA] アドレスが違っても問題です: 表には消えたメモリを指すキーが残り、TopTarget がそれを選びます。
// [JA] raw ポインターは「そのアドレス」を覚えているだけで、「そのオブジェクトがまだそのオブジェクトなのか」は知りません。
// [JA] UObject ではない場所で UObject を覚えておく必要があるとき、何を使えばよいでしょう？（UPROPERTY は使えません）
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// TWeakObjectPtr<AActor> 는 주소가 아니라 (객체 번호, 일련번호)를 기억합니다. 객체가 지워지면 IsValid() 가 false,
// Get() 이 nullptr 이 되고, 같은 자리에 새 객체가 생겨도 일련번호가 달라 다른 객체로 봅니다.
//     TMap<TWeakObjectPtr<AActor>, int32> Threat;
// TopTarget 은 IsValid() 가 아닌 키를 건너뛰고 Pair.Key.Get() 을 돌려주면 됩니다.
// (Destroy 만 하고 GC 전이어도 IsValid() 는 false 입니다.)

// [JA] TWeakObjectPtr<AActor> はアドレスではなく（オブジェクト番号、シリアル番号）を覚えます。オブジェクトが消えると IsValid() が false、
// [JA] Get() が nullptr になり、同じ場所に新しいオブジェクトができてもシリアル番号が違うので別のオブジェクトと見なします。
// [JA]     TMap<TWeakObjectPtr<AActor>, int32> Threat;
// [JA] TopTarget は IsValid() でないキーを飛ばし、Pair.Key.Get() を返せば済みます。
// [JA] （Destroy しただけで GC 前でも IsValid() は false です。）
#pragma endregion
