#include "Labs/Lab51_Glider.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab51Text
{
BH_TEXT(Plan,     "정면(+X)으로 나는 글라이더가 날개를 %d도 기울인(롤) 뒤 조종간을 당겨 기수를 %d도 든다(피치). 각각 30프레임에 나눠 입력.", "正面（+X）へ飛ぶグライダーが翼を %d 度傾けて（ロール）から、操縦桿を引いて機首を %d 度上げる（ピッチ）。それぞれ 30 フレームに分けて入力。");
BH_TEXT(Banked,   "  롤 끝: 앞 (%.2f, %.2f, %.2f), 위 (%.2f, %.2f, %.2f)", "  ロール終了: 前 (%.2f, %.2f, %.2f)、上 (%.2f, %.2f, %.2f)");
BH_TEXT(Pulled,   "  피치 끝: 앞 (%.2f, %.2f, %.2f), 오른쪽 (%.2f, %.2f, %.2f)", "  ピッチ終了: 前 (%.2f, %.2f, %.2f)、右 (%.2f, %.2f, %.2f)");
BH_TEXT(RollOnly, "롤만 했을 때 기수 방향은 그대로 (앞 방향 내적 %.3f)", "ロールだけのとき機首の向きはそのまま（前方向の内積 %.3f）");
BH_TEXT(Forward,  "조종사 기준으로 기수를 들었다: 기수가 조종사의 머리 위 쪽으로 돌았다 (기대 방향과의 내적 %.3f)", "パイロット基準で機首を上げた: 機首がパイロットの頭上の方向へ回った（期待方向との内積 %.3f）");
BH_TEXT(WingKept, "피치를 하는 동안 날개 축(오른쪽 방향)은 그대로 (내적 %.3f)", "ピッチの間、翼の軸（右方向）はそのまま（内積 %.3f）");
}

ABH_Glider::ABH_Glider()
{
	Airframe = CreateDefaultSubobject<USceneComponent>(TEXT("Airframe"));
	RootComponent = Airframe;
}

void ABH_Glider::Steer(float PitchDeg, float RollDeg)
{
	// 이번 프레임의 회전량을 지금 자세에 더한다.
	// [JA] このフレームの回転量を今の姿勢に加える。
	const FQuat Delta(FRotator(PitchDeg, 0.f, RollDeg));
	SetActorRotation(Delta * GetActorQuat());
}

void ABH_Lab51::StartLab()
{
	const int32 Roll = BugHunt::CaseInt(90, 30, 80);
	const int32 Pitch = BugHunt::CaseInt(90, 40, 90);
	BugHunt::Note(BugHunt::Fmt<Lab51Text::Plan>(Roll, Pitch));

	ABH_Glider* Glider = SpawnLabActor<ABH_Glider>(FVector(0.f, 0.f, 3000.f));
	const FVector F0 = Glider->GetActorForwardVector();

	constexpr int32 Steps = 30;
	for (int32 i = 0; i < Steps; ++i)
	{
		Glider->Steer(0.f, static_cast<float>(Roll) / Steps);
	}
	const FVector F1 = Glider->GetActorForwardVector();
	const FVector U1 = Glider->GetActorUpVector();
	const FVector R1 = Glider->GetActorRightVector();
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab51Text::Banked>(F1.X, F1.Y, F1.Z, U1.X, U1.Y, U1.Z));
	const float RollDot = FVector::DotProduct(F0, F1);
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab51Text::RollOnly>(RollDot), RollDot > 0.999f);

	for (int32 i = 0; i < Steps; ++i)
	{
		Glider->Steer(static_cast<float>(Pitch) / Steps, 0.f);
	}
	const FVector F2 = Glider->GetActorForwardVector();
	const FVector R2 = Glider->GetActorRightVector();
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab51Text::Pulled>(F2.X, F2.Y, F2.Z, R2.X, R2.Y, R2.Z));

	// 기수를 P 도 들면 앞 방향은 (롤 끝의 앞)에서 (롤 끝의 위) 쪽으로 P 도 돈다.
	// [JA] 機首を P 度上げると、前方向は（ロール終了時の前）から（ロール終了時の上）のほうへ P 度回る。
	const float P = FMath::DegreesToRadians(static_cast<float>(Pitch));
	const FVector Want = F1 * FMath::Cos(P) + U1 * FMath::Sin(P);
	const float FwdDot = FVector::DotProduct(F2, Want);
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab51Text::Forward>(FwdDot), FwdDot > 0.999f);
	const float WingDot = FVector::DotProduct(R1, R2);
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab51Text::WingKept>(WingDot), WingDot > 0.999f);

	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 수평에서 롤만 할 때는 맞습니다. 기울어진 뒤에 기수를 들면 기수가 조종사의 머리 위가 아니라 하늘(월드 +Z) 쪽으로 듭니다.
// 로그의 "롤 끝"과 "피치 끝" 벡터를 보세요. 피치를 하는 동안 날개 축(오른쪽)이 바뀌었나요?
// 피치 입력은 어느 축을 중심으로 돌았을까요? 기체의 Y 축인가요, 월드의 Y 축인가요?

// [JA] 水平からロールだけするときは正しいです。傾いた後に機首を上げると、機首がパイロットの頭上ではなく空（ワールド +Z）のほうへ上がります。
// [JA] ログの「ロール終了」と「ピッチ終了」のベクトルを見てください。ピッチの間に翼の軸（右）は変わりましたか？
// [JA] ピッチ入力はどの軸を中心に回ったのでしょう？ 機体の Y 軸ですか、ワールドの Y 軸ですか？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// FQuat 곱은 순서가 중요합니다. A * B 는 "B 를 먼저, 그다음 A" 입니다.
//   Delta * Current : Current 자세를 만든 뒤 월드 축으로 Delta 만큼 돈다 (월드 회전 = AddActorWorldRotation)
//   Current * Delta : 기체 축으로 Delta 만큼 돈 뒤 Current 를 적용한다 (로컬 회전 = AddActorLocalRotation)
// 기체가 수평이고 회전이 없을 때(Current = 항등) 두 결과가 같아서, 처음 롤 단계에서는 티가 나지 않습니다.

// [JA] FQuat の積は順序が重要です。A * B は「B を先に、次に A」です。
// [JA]   Delta * Current : Current の姿勢を作った後、ワールド軸で Delta だけ回る（ワールド回転 = AddActorWorldRotation）
// [JA]   Current * Delta : 機体の軸で Delta だけ回ってから Current を適用する（ローカル回転 = AddActorLocalRotation）
// [JA] 機体が水平で回転がないとき（Current = 単位）2 つの結果が同じなので、最初のロールの段階では目立ちません。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 조종사 기준 입력은 로컬 회전입니다.
//     SetActorRotation(GetActorQuat() * Delta);     // 또는 AddActorLocalRotation(Delta);
// FRotator 끼리 Pitch/Yaw/Roll 을 더하는 방식(Rot + Delta)도 피하세요. 오일러 각은 축이 고정 순서(Yaw → Pitch → Roll)라서
// 기울어진 자세에서는 "더한 만큼 돈다"가 성립하지 않고, 피치 ±90° 근처에서는 Yaw 와 Roll 이 같은 축이 되어(짐벌 락) 회전이 튑니다.

// [JA] パイロット基準の入力はローカル回転です。
// [JA]     SetActorRotation(GetActorQuat() * Delta);     // または AddActorLocalRotation(Delta);
// [JA] FRotator 同士で Pitch/Yaw/Roll を足すやり方（Rot + Delta）も避けてください。オイラー角は軸の順序が固定（Yaw → Pitch → Roll）なので、
// [JA] 傾いた姿勢では「足した分だけ回る」が成り立たず、ピッチ ±90° 付近では Yaw と Roll が同じ軸になって（ジンバルロック）回転が飛びます。
#pragma endregion
