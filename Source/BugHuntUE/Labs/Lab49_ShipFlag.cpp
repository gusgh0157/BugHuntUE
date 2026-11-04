#include "Labs/Lab49_ShipFlag.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab49Text
{
BH_TEXT(RaisedLog, "  %s 깃발을 올림 → 돛대 꼭대기 (%.0f, %.0f, %.0f)", "  %s の旗を掲げた → マストの先 (%.0f, %.0f, %.0f)");
BH_TEXT(Sail,      "배가 조선소(월드 원점)에서 첫 깃발을 단 뒤 동쪽 %.0f cm 의 항구로 가서 뱃머리를 %.0f도 돌리고 깃발을 바꿔 단다.", "船が造船所（ワールド原点）で最初の旗を掲げたあと、東 %.0f cm の港へ行き、船首を %.0f 度回して旗を掲げ替える。");
BH_TEXT(Shipyard,  "조선소", "造船所");
BH_TEXT(Harbor,    "항구", "港");
BH_TEXT(AtSea,     "다시 출항한 뒤", "再び出港した後");
BH_TEXT(FlagAtMast,"%s: 깃발이 돛대 꼭대기에 있다 (어긋난 거리 %.0f cm)", "%s: 旗がマストの先にある（ずれ %.0f cm）");
}

ABH_Ship::ABH_Ship()
{
	Hull = CreateDefaultSubobject<USceneComponent>(TEXT("Hull"));
	RootComponent = Hull;
	Flag = CreateDefaultSubobject<USceneComponent>(TEXT("Flag"));
	Flag->SetupAttachment(Hull);
}

void ABH_Ship::RaiseFlag(FName PortFlag)
{
	CurrentFlag = PortFlag;
	// 깃발을 내렸다가 돛대 꼭대기에 다시 올린다.
	// [JA] 旗を下ろしてから、マストの先にもう一度掲げる。
	const FVector Top = GetActorTransform().TransformPosition(MastTop);
	Flag->SetRelativeLocation(Top);
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab49Text::RaisedLog>(*PortFlag.ToString(), Top.X, Top.Y, Top.Z));
}

namespace
{
void CheckShipFlag(const ABH_Ship* Ship, const TCHAR* Where)
{
	// 돛대 꼭대기의 월드 위치 = 배의 트랜스폼으로 MastTop 을 옮긴 곳
	// [JA] マストの先のワールド位置 = 船のトランスフォームで MastTop を移した場所
	const FVector Expected = Ship->GetActorTransform().TransformPosition(Ship->MastTop);
	const float Off = FVector::Dist(Ship->Flag->GetComponentLocation(), Expected);
	BugHunt::ExpectTrue(BugHunt::Fmt<Lab49Text::FlagAtMast>(Where, Off), Off < 1.f);
}
}

void ABH_Lab49::StartLab()
{
	const float Distance = static_cast<float>(BugHunt::CaseInt(5000, 1500, 9000));
	const float Yaw = static_cast<float>(BugHunt::CaseInt(90, 30, 150));
	BugHunt::Note(BugHunt::Fmt<Lab49Text::Sail>(Distance, Yaw));

	// 조선소(월드 원점)에서 배를 띄우고 첫 깃발을 단다.
	// [JA] 造船所（ワールド原点）で船を浮かべ、最初の旗を掲げる。
	ABH_Ship* Ship = SpawnLabActor<ABH_Ship>(FVector::ZeroVector);
	Ship->RaiseFlag(TEXT("Shipyard"));
	CheckShipFlag(Ship, BugHunt::Tr<Lab49Text::Shipyard>());

	// 항구로 항해해서 뱃머리를 돌리고 항구의 깃발로 바꿔 단다.
	// [JA] 港へ航海して船首を回し、港の旗に掲げ替える。
	Ship->SetActorLocationAndRotation(FVector(Distance, 0.f, 0.f), FRotator(0.f, Yaw, 0.f));
	Ship->RaiseFlag(TEXT("Harbor"));
	CheckShipFlag(Ship, BugHunt::Tr<Lab49Text::Harbor>());

	// 다시 출항: 북쪽으로 10m
	// [JA] 再び出港: 北へ 10m
	Ship->AddActorWorldOffset(FVector(0.f, 1000.f, 0.f));
	CheckShipFlag(Ship, BugHunt::Tr<Lab49Text::AtSea>());

	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 조선소에서는 맞고 항구부터 틀립니다. 조선소와 항구에서 다른 것은 배의 위치와 방향뿐입니다.
// RaiseFlag 에 중단점을 걸고 조사식에 Top, Flag->GetRelativeLocation(), Flag->GetComponentLocation() 을 넣어 보세요.
// 로그에 찍힌 "돛대 꼭대기" 좌표는 맞나요? 그 값이 들어간 뒤 깃발의 월드 위치는요?

// [JA] 造船所では正しく、港から間違います。造船所と港で違うのは、船の位置と向きだけです。
// [JA] RaiseFlag にブレークポイントを置き、ウォッチに Top、Flag->GetRelativeLocation()、Flag->GetComponentLocation() を入れてみてください。
// [JA] ログに出た「マストの先」の座標は正しいですか？ その値が入った後の旗のワールド位置は？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// Relative 위치는 "부모 기준" 좌표입니다. 자식의 월드 위치 = 부모 트랜스폼 × Relative.
// Top 은 이미 배의 트랜스폼을 거친 월드 좌표인데, 그것을 Relative 에 넣으면 배의 트랜스폼이 한 번 더 곱해집니다.
// 부모가 원점에 회전 없이 있으면 월드 = Relative 라서 두 번 곱해도 티가 나지 않습니다. 원점에서만 테스트하면 놓치는 버그입니다.

// [JA] Relative の位置は「親基準」の座標です。子のワールド位置 = 親のトランスフォーム × Relative。
// [JA] Top はすでに船のトランスフォームを通したワールド座標なのに、それを Relative に入れると船のトランスフォームがもう一度掛かります。
// [JA] 親が原点に回転なしで置かれていればワールド = Relative なので、2 回掛けても目立ちません。原点だけでテストすると見逃すバグです。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 좌표계를 하나로 맞추세요. 배 기준 좌표를 그대로 Relative 에 넣거나, 월드 좌표를 SetWorldLocation 으로 넣습니다.
//     Flag->SetRelativeLocation(MastTop);        // 또는 Flag->SetWorldLocation(Top);
// 함수 이름의 Relative / World 와 넣는 값의 좌표계가 같은지 늘 확인하세요. 디테일 패널의 Location 옆 자물쇠 메뉴로
// 상대(Relative)/월드(World) 표시를 바꿔 보면 같은 컴포넌트의 두 값을 비교할 수 있습니다.

// [JA] 座標系を 1 つにそろえてください。船基準の座標をそのまま Relative に入れるか、ワールド座標を SetWorldLocation で入れます。
// [JA]     Flag->SetRelativeLocation(MastTop);        // または Flag->SetWorldLocation(Top);
// [JA] 関数名の Relative / World と、入れる値の座標系が同じかを常に確認してください。詳細パネルの Location の横のメニューで
// [JA] 相対（Relative）／ワールド（World）表示を切り替えると、同じコンポーネントの 2 つの値を比べられます。
#pragma endregion
