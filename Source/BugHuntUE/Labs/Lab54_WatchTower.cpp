#include "Labs/Lab54_WatchTower.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab54Text
{
BH_TEXT(SpottedLog, "  감시탑: 침입자 발견! (%s)", "  監視塔: 侵入者発見！（%s）");
BH_TEXT(Scene,      "높이 %d배의 감시탑. 침입자가 탑에서 %d cm 떨어진 곳으로 걸어 들어온다 (경보 반경 800).", "高さ %d 倍の監視塔。侵入者が塔から %d cm 離れた場所へ歩いて入ってくる（警報半径 800）。");
BH_TEXT(Radius,     "  경보 구의 실제 반경 %.0f cm (SphereRadius %.0f), 탑 높이 %.0f cm", "  警報球の実際の半径 %.0f cm（SphereRadius %.0f）、塔の高さ %.0f cm");
BH_TEXT(Height,     "탑이 기획대로 높다 (메시 높이 %.0f cm, 기대 %.0f)", "塔が企画どおり高い（メッシュの高さ %.0f cm、期待 %.0f）");
BH_TEXT(Spotted,    "경보 반경 안의 침입자를 발견했다", "警報半径内の侵入者を発見した");
}

ABH_WatchTower::ABH_WatchTower()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));

	Base = CreateDefaultSubobject<USceneComponent>(TEXT("Base"));
	RootComponent = Base;

	// 원기둥 메시는 지름 1m, 높이 1m. 바닥이 Base 에 오도록 50cm 올린다.
	// [JA] 円柱メッシュは直径 1m、高さ 1m。底が Base に来るよう 50cm 上げる。
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(Base);
	Mesh->SetStaticMesh(Cylinder.Object);
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Alarm = CreateDefaultSubobject<USphereComponent>(TEXT("Alarm"));
	Alarm->SetupAttachment(Base);
	Alarm->InitSphereRadius(800.f);
	Alarm->SetCollisionProfileName(TEXT("Trigger"));
	Alarm->SetGenerateOverlapEvents(true);
	Alarm->OnComponentBeginOverlap.AddDynamic(this, &ABH_WatchTower::OnSpotted);
}

void ABH_WatchTower::Build(float HeightScale)
{
	// 가늘고 높은 탑
	// [JA] 細くて高い塔
	SetActorScale3D(FVector(0.5f, 0.5f, HeightScale));
	Mesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
}

void ABH_WatchTower::OnSpotted(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor && OtherActor->IsA<ABH_Intruder>())
	{
		++Alarms;
		UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab54Text::SpottedLog>(*OtherActor->GetName()));
	}
}

ABH_Intruder::ABH_Intruder()
{
	Body = CreateDefaultSubobject<USphereComponent>(TEXT("Body"));
	Body->InitSphereRadius(30.f);
	Body->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	Body->SetGenerateOverlapEvents(true);
	RootComponent = Body;
}

void ABH_Lab54::StartLab()
{
	const int32 Height = BugHunt::CaseInt(4, 3, 6);
	const int32 Distance = BugHunt::CaseInt(600, 450, 740);
	BugHunt::Note(BugHunt::Fmt<Lab54Text::Scene>(Height, Distance));

	const FVector Site(4000.f, 4000.f, 1000.f);
	ABH_WatchTower* Tower = SpawnLabActor<ABH_WatchTower>(Site);
	Tower->Build(static_cast<float>(Height));

	FVector Origin, Extent;
	Tower->Mesh->Bounds.GetBox().GetCenterAndExtents(Origin, Extent);
	const float MeshHeight = Extent.Z * 2.f;
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab54Text::Radius>(Tower->Alarm->GetScaledSphereRadius(), Tower->Alarm->GetUnscaledSphereRadius(), MeshHeight));
	const float WantHeight = 100.f * static_cast<float>(Height);
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab54Text::Height>(MeshHeight, WantHeight), FMath::IsNearlyEqual(MeshHeight, WantHeight, 2.f));

	// 침입자는 멀리서 출발해서 Distance 까지 걸어온다 (멈춘 곳에서 탑 아래쪽과 같은 높이).
	// [JA] 侵入者は遠くから出発して Distance まで歩いてくる（止まった場所で塔の下と同じ高さ）。
	ABH_Intruder* Intruder = SpawnLabActor<ABH_Intruder>(Site + FVector(3000.f, 0.f, 100.f));
	Intruder->SetActorLocation(Site + FVector(static_cast<float>(Distance), 0.f, 100.f));

	After(0.5f, [this, Tower]
	{
		BugHunt::ExpectTrue(BugHunt::Tr<Lab54Text::Spotted>(), Tower->Alarms >= 1);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 경보 반경은 800 이라고 적혀 있는데, 600 cm 앞의 침입자를 못 봅니다. 로그에 찍힌 "경보 구의 실제 반경"을 보세요.
// PIE 에서 콘솔에 show Collision 을 넣으면 경보 구가 실제로 얼마나 큰지 보입니다.
// SphereRadius 는 800 그대로입니다. 그러면 실제 반경을 바꾼 것은 무엇일까요?

// [JA] 警報半径は 800 と書いてあるのに、600 cm 先の侵入者に気づきません。ログに出た「警報球の実際の半径」を見てください。
// [JA] PIE でコンソールに show Collision と入れると、警報球が実際にどれくらいの大きさか見えます。
// [JA] SphereRadius は 800 のままです。では、実際の半径を変えたのは何でしょう？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 자식 컴포넌트는 부모의 스케일을 물려받습니다. Build 는 탑 모양을 내려고 액터(루트) 스케일을 (0.5, 0.5, 높이) 로 바꿨고,
// 그 아래에 붙은 경보 구도 같은 스케일을 받습니다. 구는 찌그러질 수 없어서 엔진은 세 축 중 가장 작은 스케일을 씁니다
// (USphereComponent::GetScaledSphereRadius = SphereRadius × 가장 작은 축 스케일). 캡슐·박스도 스케일에 따라 범위가 바뀝니다.

// [JA] 子コンポーネントは親のスケールを受け継ぎます。Build は塔の形にするため、アクター（ルート）のスケールを (0.5, 0.5, 高さ) に変えました。
// [JA] その下に付いた警報球も同じスケールを受けます。球は歪められないので、エンジンは 3 軸のうち最も小さいスケールを使います
// [JA] （USphereComponent::GetScaledSphereRadius = SphereRadius × 最も小さい軸のスケール）。カプセルやボックスもスケールによって範囲が変わります。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 모양을 바꾸는 스케일은 모양을 가진 컴포넌트(메시)에만 줍니다. 루트와 다른 자식은 스케일 1 로 둡니다.
//     Mesh->SetRelativeScale3D(FVector(0.5f, 0.5f, HeightScale));
//     Mesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f * HeightScale));   // 바닥을 Base 에 맞춘다
// 루트 스케일을 꼭 써야 하면 범위 컴포넌트에 SetUsingAbsoluteScale(true) 를 주어 부모 스케일을 끊는 방법도 있습니다.

// [JA] 形を変えるスケールは、形を持つコンポーネント（メッシュ）にだけ与えます。ルートとほかの子はスケール 1 のままにします。
// [JA]     Mesh->SetRelativeScale3D(FVector(0.5f, 0.5f, HeightScale));
// [JA]     Mesh->SetRelativeLocation(FVector(0.f, 0.f, 50.f * HeightScale));   // 底を Base に合わせる
// [JA] どうしてもルートのスケールを使う場合は、範囲用のコンポーネントに SetUsingAbsoluteScale(true) を与えて親のスケールを切る方法もあります。
#pragma endregion
