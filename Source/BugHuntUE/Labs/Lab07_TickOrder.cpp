#include "Labs/Lab07_TickOrder.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab07Text
{
BH_TEXT(Start,     "영웅 HP %d, 매 프레임 1씩 출혈", "英雄の HP %d、毎フレーム 1 ずつ出血");
BH_TEXT(Mismatch,  "  프레임 %llu: 실제 HP %d, HP 바 %d", "  フレーム %llu: 実際の HP %d、HP バー %d");
BH_TEXT(WrongNum,  "HP 바가 실제 HP와 다른 프레임 수", "HP バーが実際の HP と違ったフレーム数");
BH_TEXT(BarAtDeath, "영웅이 쓰러진 프레임의 HP 바", "英雄が倒れたフレームの HP バー");
}

ABH_BleedingHero::ABH_BleedingHero()
{
	PrimaryActorTick.bCanEverTick = true;
	// 이동/애니메이션이 다 끝난 뒤에 상태 이상을 처리한다.
	// [JA] 移動／アニメーションがすべて終わった後に状態異常を処理する。
	PrimaryActorTick.TickGroup = TG_PostUpdateWork;
}

void ABH_BleedingHero::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Health > 0)
	{
		--Health;
	}
}

ABH_HealthBar::ABH_HealthBar()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ABH_HealthBar::Attach(ABH_BleedingHero* InHero)
{
	Hero = InHero;
	DisplayedHealth = Hero->Health;
}

void ABH_HealthBar::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (Hero)
	{
		DisplayedHealth = Hero->Health;
	}
}

// ---------------------------------------------------------------------------
// 아래는 검사 코드입니다. 버그는 위쪽에 있습니다.
// [JA] 以下は検査コードです。バグは上のほうにあります。

ABH_Lab07::ABH_Lab07()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;
	PrimaryActorTick.TickGroup = TG_LastDemotable;
}

void ABH_Lab07::StartLab()
{
	Hero = SpawnLabActor<ABH_BleedingHero>();
	Bar = SpawnLabActor<ABH_HealthBar>();
	Bar->Attach(Hero);
	BugHunt::Note(BugHunt::Fmt<Lab07Text::Start>(Hero->Health));
	SetActorTickEnabled(true);
}

void ABH_Lab07::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	++FramesChecked;
	if (Bar->DisplayedHealth != Hero->Health)
	{
		++FramesWrong;
		UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab07Text::Mismatch>(GFrameCounter, Hero->Health, Bar->DisplayedHealth));
	}
	if (Hero->Health == 0 && BarWhenHeroDied < 0)
	{
		BarWhenHeroDied = Bar->DisplayedHealth;
	}

	if (FramesChecked >= 40)
	{
		SetActorTickEnabled(false);
		BugHunt::Expect(BugHunt::Tr<Lab07Text::WrongNum>(), 0, FramesWrong);
		BugHunt::Expect(BugHunt::Tr<Lab07Text::BarAtDeath>(), 0, BarWhenHeroDied);
		FinishLab();
	}
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// ABH_BleedingHero::Tick 과 ABH_HealthBar::Tick 에 추적점(중단점 우클릭 → 작업, "실행 계속" 체크)을 걸고
//     Hero {Health}      /      Bar reads {Hero->Health}
// 를 출력해 보세요. 출력 창(Ctrl+Alt+O)에서 둘 중 누가 먼저 찍히나요? 매 프레임 같은 순서인가요?
// 콘솔(~)에 dumpticks 를 입력하면 등록된 Tick 함수와 Tick 그룹 목록이 Output Log에 나옵니다.

// [JA] ABH_BleedingHero::Tick と ABH_HealthBar::Tick にトレースポイント（ブレークポイントを右クリック → アクション、「実行の継続」にチェック）を設定し、
// [JA]     Hero {Health}      /      Bar reads {Hero->Health}
// [JA] を出力してみてください。出力 ウィンドウ（Ctrl+Alt+O）で、2 つのうちどちらが先に出力されますか？ 毎フレーム同じ順番ですか？
// [JA] コンソール（~）で dumpticks を入力すると、登録された Tick 関数と Tick グループの一覧がアウトプットログに出ます。
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 언리얼은 한 프레임을 Tick 그룹 순서대로 돕니다:
//     TG_PrePhysics → TG_DuringPhysics → TG_PostPhysics → TG_PostUpdateWork → TG_LastDemotable
// 액터의 기본 그룹은 TG_PrePhysics 입니다. 영웅과 HP 바는 각각 어느 그룹인가요?

// [JA] Unreal は 1 フレームを Tick グループの順番に回します:
// [JA]     TG_PrePhysics → TG_DuringPhysics → TG_PostPhysics → TG_PostUpdateWork → TG_LastDemotable
// [JA] アクターの既定のグループは TG_PrePhysics です。英雄と HP バーはそれぞれどのグループですか？
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// "A 를 읽는 쪽은 A 를 바꾸는 쪽보다 뒤에 Tick 해야 한다."
// 같은 그룹 안에서의 순서는 보장되지 않습니다. 순서가 중요하면 AddTickPrerequisiteActor 로 명시하세요.

// [JA] 「A を読む側は、A を変える側より後に Tick しなければならない。」
// [JA] 同じグループの中での順番は保証されません。順番が重要なら AddTickPrerequisiteActor で明示してください。
#pragma endregion
