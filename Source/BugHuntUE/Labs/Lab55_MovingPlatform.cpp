#include "Labs/Lab55_MovingPlatform.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntProbe.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab55Text
{
BH_TEXT(Scene,  "발판이 초속 %d cm 로 +Y 쪽으로 2초 움직인다.", "足場が秒速 %d cm で +Y のほうへ 2 秒動く。");
BH_TEXT(Moved,  "  2초 뒤 발판 위치 Y=%.0f (출발 Y=%.0f)", "  2 秒後の足場の位置 Y=%.0f（出発 Y=%.0f）");
BH_TEXT(Travel, "발판이 움직인 거리 (cm)", "足場が動いた距離（cm）");
}

ABH_MovingPlatform::ABH_MovingPlatform()
{
	PrimaryActorTick.bCanEverTick = true;

	// 레벨의 벽·바닥과 같은 설정 (조명·내비 빌드가 빠르도록 Static)
	// [JA] レベルの壁・床と同じ設定（ライティング・ナビのビルドが速くなるよう Static）
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetMobility(EComponentMobility::Static);
	Mesh->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	Mesh->SetRelativeScale3D(FVector(3.f, 3.f, 0.2f));
	RootComponent = Mesh;
}

void ABH_MovingPlatform::BeginPlay()
{
	Super::BeginPlay();
	Start = GetActorLocation();
}

void ABH_MovingPlatform::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetActorLocation(GetActorLocation() + FVector(0.f, Speed * DeltaSeconds, 0.f));
}

void ABH_Lab55::StartLab()
{
	BugHunt::SetFixedFrameRate(60.f);

	const int32 Speed = BugHunt::CaseInt(200, 120, 300);
	BugHunt::Note(BugHunt::Fmt<Lab55Text::Scene>(Speed));

	ABH_MovingPlatform* Platform = SpawnLabActor<ABH_MovingPlatform>(FVector(-4000.f, 2000.f, 1500.f));
	Platform->Speed = static_cast<float>(Speed);
	const float StartY = Platform->GetActorLocation().Y;

	After(2.f, [this, Platform, StartY, Speed]
	{
		const float Y = Platform->GetActorLocation().Y;
		UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab55Text::Moved>(Y, StartY));
		// 2초 동안 Speed × 2 cm. 타이머와 Tick 순서 때문에 한두 프레임 차이는 봐준다.
		// [JA] 2 秒間で Speed × 2 cm。タイマーと Tick の順序のため、1〜2 フレームの差は許容する。
		const int32 Travel = FMath::RoundToInt(Y - StartY);
		const int32 Want = Speed * 2;
		const int32 Frame = FMath::CeilToInt(static_cast<float>(Speed) / 60.f);
		BugHunt::Expect(BugHunt::Tr<Lab55Text::Travel>(), Want, FMath::Abs(Travel - Want) <= 2 * Frame ? Want : Travel);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// Tick 은 매 프레임 불리고(중단점을 걸어 보세요) SetActorLocation 도 불리는데 발판은 꼼짝하지 않습니다. 크래시도 에러도 없습니다.
// SetActorLocation 은 bool 을 돌려줍니다. 조사식이나 지역 변수로 그 값을 받아 보세요.
// 그리고 출력 로그(Output Log)와 메시지 로그(Window → Message Log 의 PIE 탭)에 경고가 없는지 보세요.

// [JA] Tick は毎フレーム呼ばれ（ブレークポイントを置いてみてください）SetActorLocation も呼ばれているのに、足場はびくともしません。クラッシュもエラーもありません。
// [JA] SetActorLocation は bool を返します。ウォッチやローカル変数でその値を受け取ってみてください。
// [JA] そしてアウトプットログ（Output Log）とメッセージログ（Window → Message Log の PIE タブ）に警告がないか見てください。
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 컴포넌트의 Mobility 가 Static 이면 게임 중에는 옮길 수 없습니다. 엔진은 이동 요청을 무시하고 false 를 돌려주며
// "Mobility of ... has to be 'Movable' if you'd like to move" 경고만 남깁니다 (USceneComponent::CheckStaticMobilityAndWarn).
// 스폰할 때 처음 위치를 정하는 것은 되기 때문에(액터 초기화 전) "놓이기는 하는데 안 움직이는" 모양이 됩니다.

// [JA] コンポーネントの Mobility が Static なら、ゲーム中には動かせません。エンジンは移動の要求を無視して false を返し、
// [JA] 「Mobility of ... has to be 'Movable' if you'd like to move」という警告だけを残します（USceneComponent::CheckStaticMobilityAndWarn）。
// [JA] スポーン時に最初の位置を決めることはできる（アクターの初期化前）ので、「置かれはするが動かない」という形になります。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 게임 중에 움직이는 것은 Movable 로 만듭니다.
//     Mesh->SetMobility(EComponentMobility::Movable);
// Static 은 미리 구운 조명·그림자·내비게이션을 위한 것이라 문, 엘리베이터, 발판처럼 움직이는 것에는 쓰지 않습니다.
// 이동 함수의 반환값(SetActorLocation, SetWorldLocation 의 bool, 스윕의 FHitResult)을 확인하는 습관도 들이세요.

// [JA] ゲーム中に動くものは Movable にします。
// [JA]     Mesh->SetMobility(EComponentMobility::Movable);
// [JA] Static は事前に焼いたライティング・影・ナビゲーションのためのもので、扉、エレベーター、足場のように動くものには使いません。
// [JA] 移動関数の戻り値（SetActorLocation、SetWorldLocation の bool、スイープの FHitResult）を確認する習慣もつけてください。
#pragma endregion
