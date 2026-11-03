#include "Labs/Lab01_Pickup.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/SphereComponent.h"
#include "Engine/CollisionProfile.h"

// 회복 아이템: 영웅이 닿으면 HP를 30 회복시키고 사라진다.
// 기획 메모: 펫이 주운 회복 아이템은 주인(영웅)에게 간다.

ABH_Hero::ABH_Hero()
{
	Body = CreateDefaultSubobject<USphereComponent>(TEXT("Body"));
	Body->InitSphereRadius(40.f);
	Body->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	Body->SetGenerateOverlapEvents(true);
	RootComponent = Body;
// [JA] 回復アイテム: 英雄が触れると HP を 30 回復させて消える。
// [JA] 企画メモ: ペットが拾った回復アイテムは主人（英雄）に渡る。
}

void ABH_Hero::Heal(int32 Amount)
{
	Health = FMath::Min(MaxHealth, Health + Amount);
	UE_LOG(LogBugHunt, Log, TEXT("  %s 회복 +%d -> HP %d"), *GetName(), Amount, Health);
}

ABH_Pet::ABH_Pet()
{
	Body = CreateDefaultSubobject<USphereComponent>(TEXT("Body"));
	Body->InitSphereRadius(25.f);
	Body->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);
	Body->SetGenerateOverlapEvents(true);
	RootComponent = Body;
}

ABH_HealthPickup::ABH_HealthPickup()
{
	Trigger = CreateDefaultSubobject<USphereComponent>(TEXT("Trigger"));
	Trigger->InitSphereRadius(50.f);
	Trigger->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Trigger->SetGenerateOverlapEvents(true);
	RootComponent = Trigger;
}

void ABH_HealthPickup::BeginPlay()
{
	Super::BeginPlay();
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ABH_HealthPickup::HandleOverlap);
}

void ABH_HealthPickup::HandleOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	OnTouched(OtherActor);
}

void ABH_HealthPickup::OnTouched(AActor* Toucher)
{
	ABH_Hero* Hero = Cast<ABH_Hero>(Toucher);
	GiveHealth(Hero);
}

void ABH_HealthPickup::GiveHealth(ABH_Hero* Hero)
{
	Hero->Heal(HealAmount);
	Destroy();
}

void ABH_Lab01::StartLab()
{
	// 아이템 세 개를 길에 놓는다.
	SpawnLabActor<ABH_HealthPickup>(FVector(300.f, 0.f, 100.f));
	SpawnLabActor<ABH_HealthPickup>(FVector(600.f, 0.f, 100.f));
	SpawnLabActor<ABH_HealthPickup>(FVector(900.f, 0.f, 100.f));

	ABH_Hero* Hero = SpawnLabActor<ABH_Hero>(FVector(0.f, 0.f, 100.f));
	ABH_Pet* Pet = SpawnLabActor<ABH_Pet>(FVector(0.f, -300.f, 100.f));
	Pet->Master = Hero;

	// [JA] アイテムを 3 つ道に置く。
	BugHunt::Note(FString::Printf(TEXT("시작 HP %d. 영웅이 1번, 펫이 2번, 영웅이 3번 아이템을 줍는다."), Hero->Health));

	// 녹화해 둔 이동을 재생한다. 아이템 위로 옮기는 순간 겹침(Overlap) 이벤트가 난다.
	Hero->SetActorLocation(FVector(300.f, 0.f, 100.f));
	Pet->SetActorLocation(FVector(600.f, 0.f, 100.f));
	Hero->SetActorLocation(FVector(900.f, 0.f, 100.f));

	BugHunt::Expect(TEXT("아이템 세 개를 다 먹은 영웅의 HP"), 100, Hero->Health);
	FinishLab();
}

	// [JA] 録画しておいた移動を再生する。アイテムの上へ動かした瞬間にオーバーラップ（Overlap）イベントが起きる。
// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 호출 스택을 보고 펼치세요 / まず呼び出し履歴を見てから開いてください)
// 디버거가 "읽기 액세스 위반... this이(가) nullptr였습니다" 비슷한 메시지로 멈춥니다.
// 호출 스택(Ctrl+Alt+C) 맨 위 ABH_Hero::Heal 에서 this 가 0x0000000000000000 입니다.
// nullptr 인 영웅에게 Heal 을 부른 것입니다. 한 칸씩 아래 프레임을 더블클릭하세요.
#pragma endregion

// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
#pragma region 힌트 2 / ヒント 2
// GiveHealth → OnTouched 로 내려가면 Toucher 는 nullptr 가 아닙니다. 지역 창에서 Toucher 를 펼쳐
// 이름(Name)이나 클래스를 보세요. 누가 아이템을 건드렸나요? 그때 Cast<ABH_Hero> 의 결과는?
// 더 아래에는 UPrimitiveComponent::BeginComponentOverlap 같은 엔진 프레임이 보입니다.
// 엔진 코드는 건너뛰고 ABH_Lab01::StartLab 까지 내려가면 "어느 이동"에서 터졌는지 알 수 있습니다.
#pragma endregion

// [JA] デバッガーが「読み取りアクセス違反... this は nullptr でした」のようなメッセージで止まります。
// [JA] 呼び出し履歴（Ctrl+Alt+C）のいちばん上、ABH_Hero::Heal で this が 0x0000000000000000 です。
// [JA] nullptr の英雄に Heal を呼んだのです。1 つずつ下のフレームをダブルクリックしてください。

#pragma region 힌트 3 / ヒント 3
// Cast<T>는 실패하면 크래시 대신 nullptr 를 돌려줍니다. 돌려받은 쪽이 검사해야 합니다.
// 단, if (!Hero) return; 만 넣으면 크래시는 사라지지만 [FAIL] 이 남습니다. 파일 맨 위 기획 메모를 다시 읽으세요.
#pragma endregion

// [JA] GiveHealth → OnTouched と下りると、Toucher は nullptr ではありません。ローカル ウィンドウで Toucher を展開し、
// [JA] 名前（Name）やクラスを見てください。誰がアイテムに触れましたか？ そのときの Cast<ABH_Hero> の結果は？
// [JA] さらに下には UPrimitiveComponent::BeginComponentOverlap のようなエンジンのフレームが見えます。
// [JA] エンジンのコードは飛ばして ABH_Lab01::StartLab まで下りると、「どの移動」で落ちたのかがわかります。

// [JA] Cast<T> は失敗するとクラッシュの代わりに nullptr を返します。受け取った側が検査しなければなりません。
// [JA] ただし、if (!Hero) return; だけを入れるとクラッシュは消えますが [FAIL] が残ります。ファイル冒頭の企画メモを読み直してください。
