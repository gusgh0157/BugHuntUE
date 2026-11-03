#include "Labs/Lab26_PressurePlate.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ABH_PressurePlate::ABH_PressurePlate()
{
	Trigger = CreateDefaultSubobject<UBoxComponent>(TEXT("Trigger"));
	Trigger->InitBoxExtent(FVector(100.f, 100.f, 30.f));
	Trigger->SetCollisionProfileName(TEXT("Trigger"));   // 모든 채널에 Overlap, 물리 충돌 없음
	Trigger->SetGenerateOverlapEvents(true);
	Trigger->OnComponentBeginOverlap.AddDynamic(this, &ABH_PressurePlate::OnPressed);
	RootComponent = Trigger;
}

void ABH_PressurePlate::OnPressed(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	++Presses;
	bDoorOpen = true;
	UE_LOG(LogBugHunt, Log, TEXT("  압력판 눌림: %s → 문 열림"), *GetNameSafe(OtherActor));
}
	// [JA] すべてのチャンネルに Overlap、物理衝突なし

ABH_Ground::ABH_Ground()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->InitBoxExtent(FVector(1000.f, 1000.f, 10.f));
	Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	RootComponent = Box;
}

ABH_WoodCrate::ABH_WoodCrate()
{
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	UStaticMeshComponent* Mesh = GetStaticMeshComponent();
	Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetMobility(EComponentMobility::Movable);
	Mesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
	Mesh->SetSimulatePhysics(true);
	Mesh->SetRelativeScale3D(FVector(0.5f));            // 50cm 상자
}

void ABH_Lab26::StartLab()
{
	BugHunt::SetFixedFrameRate(60.f);

	// 기본 맵의 바닥과 플레이어에서 떨어진 빈 공중에 바닥을 깐다. 바닥 윗면 = Base.Z, 압력판은 그 위 2~62cm 공간.
	const FVector Base(0.f, 3000.f, 2000.f);
	SpawnLabActor<ABH_Ground>(Base + FVector(0.f, 0.f, -10.f));
	ABH_PressurePlate* Plate = SpawnLabActor<ABH_PressurePlate>(Base + FVector(0.f, 0.f, 32.f));
	const float DropHeight = static_cast<float>(BugHunt::CaseInt(300, 150, 600));
	// [JA] 50cm の箱
	ABH_WoodCrate* Crate = SpawnLabActor<ABH_WoodCrate>(Base + FVector(0.f, 0.f, DropHeight));
	BugHunt::Note(FString::Printf(TEXT("압력판 위 %.0f cm 에서 상자를 떨어뜨린다."), DropHeight));

	After(2.5f, [this, Plate, Crate, Base]
	{
		BugHunt::Note(FString::Printf(TEXT("상자 위치 Z=%.0f (바닥에 놓이면 25)"), Crate->GetActorLocation().Z - Base.Z));
		BugHunt::ExpectTrue(TEXT("상자가 압력판 위에 놓였다 (Z < 60)"), Crate->GetActorLocation().Z - Base.Z < 60.f);
	// [JA] 既定マップの床とプレイヤーから離れた、何もない空中に床を敷く。床の上面 = Base.Z、感圧板はその上 2〜62cm の空間。
		BugHunt::Expect(TEXT("압력판이 눌린 횟수"), 1, Plate->Presses);
		BugHunt::ExpectTrue(TEXT("문이 열렸다"), Plate->bDoorOpen);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 상자는 압력판 안에 들어가 있는데(show Collision 으로 보세요) OnPressed 가 한 번도 불리지 않습니다.
// PIE 를 일시 정지하고 World Outliner 에서 상자와 압력판을 각각 골라 디테일 패널 Collision 칸을 나란히 비교하세요.
// Collision Presets 의 채널 응답과 그 위의 체크박스들을 보세요.
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// Overlap 이벤트는 두 컴포넌트가 다음을 모두 만족할 때만 생깁니다:
//   ① 서로의 오브젝트 채널에 대한 응답이 둘 다 Overlap 이상 (한쪽이라도 Ignore 면 없음, 둘 다 Block 이면 Overlap 대신 Hit)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
//   ② 둘 다 Generate Overlap Events 가 켜져 있음
// 압력판은 직접 켰습니다. 상자는 생성자에서 아무것도 안 바꿨는데... 부모 클래스 생성자가 무엇을 하는지 F12 로 들어가 보세요.
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// AStaticMeshActor 생성자는 성능을 위해 Generate Overlap Events 를 끕니다(레벨에 깔리는 수천 개의 벽과 바닥 때문).

// [JA] 箱は感圧板の中に入っているのに（show Collision で見てください）、OnPressed が一度も呼ばれません。
// [JA] PIE を一時停止し、アウトライナで箱と感圧板をそれぞれ選んで、詳細パネルの Collision 欄を並べて比べてください。
// [JA] Collision Presets のチャンネル応答と、その上のチェックボックスを見てください。
// 상자처럼 트리거와 상호작용해야 하는 것은 다시 켭니다:
//     Mesh->SetGenerateOverlapEvents(true);
// Block 이 필요한 것(문, 벽)과 Overlap 이 필요한 것(발판, 트리거)은 채널 응답으로 나눕니다. 둘 다 Block 이면 OnComponentHit
// (그리고 Simulation Generates Hit Events) 쪽으로 옵니다.
#pragma endregion

// [JA] Overlap イベントは、2 つのコンポーネントが次のすべてを満たすときにだけ発生します:
// [JA]   ① 互いのオブジェクト チャンネルに対する応答が両方とも Overlap 以上（片方でも Ignore ならなし、両方 Block なら Overlap の代わりに Hit）
// [JA]   ② 両方とも Generate Overlap Events がオンになっている
// [JA] 感圧板は自分でオンにしました。箱はコンストラクターで何も変えていないのに... 親クラスのコンストラクターが何をしているか、F12 で入ってみてください。

// [JA] AStaticMeshActor のコンストラクターは、パフォーマンスのために Generate Overlap Events をオフにします（レベルに敷かれる何千もの壁や床のため）。
// [JA] 箱のようにトリガーと相互作用する必要があるものは、オンに戻します:
// [JA]     Mesh->SetGenerateOverlapEvents(true);
// [JA] Block が必要なもの（扉、壁）と Overlap が必要なもの（踏み板、トリガー）は、チャンネル応答で分けます。両方 Block なら OnComponentHit
// [JA] （そして Simulation Generates Hit Events）のほうに来ます。
