#include "Labs/Lab53_LaserSight.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab53Text
{
BH_TEXT(Scene,    "전차(스케일 %.1f)가 (%d, %d) 에서 %d도 방향을 본다.", "戦車（スケール %.1f）が (%d, %d) で %d 度の方向を向いている。");
BH_TEXT(LaserLog, "  레이저: 시작 (%.0f, %.0f, %.0f), 방향 (%.2f, %.2f, %.2f)", "  レーザー: 始点 (%.0f, %.0f, %.0f)、方向 (%.2f, %.2f, %.2f)");
BH_TEXT(Start,    "레이저가 포구에서 나간다 (포구와의 거리 %.0f cm)", "レーザーが砲口から出ている（砲口との距離 %.0f cm）");
BH_TEXT(Dir,      "레이저가 포신 방향을 가리킨다 (내적 %.3f)", "レーザーが砲身の向きを指している（内積 %.3f）");
}

ABH_Tank::ABH_Tank()
{
	Hull = CreateDefaultSubobject<USceneComponent>(TEXT("Hull"));
	RootComponent = Hull;
	MuzzleMarker = CreateDefaultSubobject<USceneComponent>(TEXT("MuzzleMarker"));
	MuzzleMarker->SetupAttachment(Hull);
	MuzzleMarker->SetRelativeLocation(MuzzleOffset);
}

void ABH_Tank::UpdateLaser()
{
	// 전차 기준 좌표를 월드로 바꾼다: 포구 위치, 그리고 포신이 보는 방향(전차 기준 +X).
	// [JA] 戦車基準の座標をワールドに変換する: 砲口の位置と、砲身が向いている方向（戦車基準の +X）。
	const FTransform& ToWorld = GetActorTransform();
	LaserStart = ToWorld.TransformVector(MuzzleOffset);
	LaserDir = ToWorld.TransformPosition(FVector::ForwardVector).GetSafeNormal();
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab53Text::LaserLog>(LaserStart.X, LaserStart.Y, LaserStart.Z, LaserDir.X, LaserDir.Y, LaserDir.Z));
}

void ABH_Lab53::StartLab()
{
	const int32 X = BugHunt::CaseInt(2000, -4000, 4000);
	const int32 Y = BugHunt::CaseInt(-1500, -4000, 4000);
	const int32 Yaw = BugHunt::CaseInt(135, 20, 340);
	const float Scale = static_cast<float>(BugHunt::CaseInt(15, 10, 20)) / 10.f;
	BugHunt::Note(BugHunt::Fmt<Lab53Text::Scene>(Scale, X, Y, Yaw));

	ABH_Tank* Tank = SpawnLabActor<ABH_Tank>(FVector(static_cast<float>(X), static_cast<float>(Y), 0.f));
	Tank->SetActorRotation(FRotator(0.f, static_cast<float>(Yaw), 0.f));
	Tank->SetActorScale3D(FVector(Scale));
	Tank->UpdateLaser();

	// 표시용 포구 컴포넌트로 확인한다 (월드 위치·방향은 엔진이 계산한 값).
	// [JA] 表示用の砲口コンポーネントで確認する（ワールド位置・向きはエンジンが計算した値）。
	const float StartOff = FVector::Dist(Tank->LaserStart, Tank->MuzzleMarker->GetComponentLocation());
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab53Text::Start>(StartOff), StartOff < 1.f);
	const float DirDot = FVector::DotProduct(Tank->LaserDir, Tank->MuzzleMarker->GetForwardVector());
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab53Text::Dir>(DirDot), DirDot > 0.999f);

	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 레이저 시작점은 전차가 어디에 있든 월드 원점 근처입니다. 방향은 전차가 원점에서 멀수록 엉뚱해집니다.
// UpdateLaser 에서 ToWorld 를 조사식에 넣어 Translation, Rotation, Scale3D 를 보세요.
// 포구 위치 계산에는 Translation 이 들어갔나요? 방향 계산에는요? 어느 쪽에 들어가야 할까요?

// [JA] レーザーの始点は、戦車がどこにいてもワールド原点の近くです。方向は、戦車が原点から遠いほどおかしくなります。
// [JA] UpdateLaser で ToWorld をウォッチに入れて、Translation、Rotation、Scale3D を見てください。
// [JA] 砲口の位置の計算に Translation は入りましたか？ 方向の計算には？ どちらに入るべきでしょう？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// FTransform 의 변환 함수는 "점"과 "방향"을 구분합니다.
//   TransformPosition       : 스케일 → 회전 → 평행이동 (점. 위치는 전차를 따라 옮겨져야 한다)
//   TransformVector         : 스케일 → 회전 (방향·변위. 평행이동 없음. 길이는 스케일만큼 바뀐다)
//   TransformVectorNoScale  : 회전만 (단위 방향을 그대로 단위 방향으로)
// 여기서는 두 줄이 서로 바뀌어 있습니다. 전차가 원점에 회전·스케일 없이 있으면 셋 다 같은 값을 돌려줘서 티가 나지 않습니다.

// [JA] FTransform の変換関数は「点」と「方向」を区別します。
// [JA]   TransformPosition       : スケール → 回転 → 平行移動（点。位置は戦車に合わせて移されなければならない）
// [JA]   TransformVector         : スケール → 回転（方向・変位。平行移動なし。長さはスケールの分だけ変わる）
// [JA]   TransformVectorNoScale  : 回転のみ（単位方向をそのまま単位方向に）
// [JA] ここでは 2 行が入れ替わっています。戦車が原点に回転・スケールなしで置かれていれば 3 つとも同じ値を返すので、目立ちません。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 점은 TransformPosition, 방향은 TransformVectorNoScale 입니다.
//     LaserStart = ToWorld.TransformPosition(MuzzleOffset);
//     LaserDir = ToWorld.TransformVectorNoScale(FVector::ForwardVector);
// 반대 방향(월드 → 전차 기준)은 InverseTransformPosition / InverseTransformVectorNoScale 입니다.
// 방향에 평행이동이 섞이면 "원점에서 멀어질수록 더 틀린다"는 모양으로 나타납니다.

// [JA] 点は TransformPosition、方向は TransformVectorNoScale です。
// [JA]     LaserStart = ToWorld.TransformPosition(MuzzleOffset);
// [JA]     LaserDir = ToWorld.TransformVectorNoScale(FVector::ForwardVector);
// [JA] 逆方向（ワールド → 戦車基準）は InverseTransformPosition / InverseTransformVectorNoScale です。
// [JA] 方向に平行移動が混ざると、「原点から離れるほど大きく間違う」という形で現れます。
#pragma endregion
