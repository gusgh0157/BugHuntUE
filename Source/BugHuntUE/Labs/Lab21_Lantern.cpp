#include "Labs/Lab21_Lantern.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/SkeletalMeshComponent.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab21Text
{
BH_TEXT(HeldLog,  "  랜턴을 %s 에 붙임 → 랜턴 위치 (%.0f, %.0f, %.0f)", "  ランタンを %s に付けた → ランタンの位置 (%.0f, %.0f, %.0f)");
BH_TEXT(Scene,    "광부가 (%d, %d) 에서 %d도 방향을 보고 서서 오른손에 랜턴을 든다.", "鉱夫が (%d, %d) で %d 度の方向を向いて立ち、右手にランタンを持つ。");
BH_TEXT(Raised,   "랜턴이 발밑이 아니라 손 높이에 있다 (발에서 %.0f cm 위, 기준 80)", "ランタンが足元ではなく手の高さにある（足から %.0f cm 上、基準 80）");
BH_TEXT(InHand,   "랜턴이 오른손(hand_r 뼈)에 있다 (거리 %.0f cm)", "ランタンが右手（hand_r ボーン）にある（距離 %.0f cm）");
}

ABH_Lantern::ABH_Lantern()
{
	Light = CreateDefaultSubobject<USceneComponent>(TEXT("Light"));
	RootComponent = Light;
}

void ABH_Miner::HoldLantern(ABH_Lantern* Lantern)
{
	// 손 소켓에 맞춰 붙인다.
	// [JA] 手のソケットに合わせて付ける。
	Lantern->AttachToComponent(Mesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, LanternSocket);
	const FVector At = Lantern->GetActorLocation();
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab21Text::HeldLog>(*LanternSocket.ToString(), At.X, At.Y, At.Z));
}

void ABH_Lab21::StartLab()
{
	const int32 X = BugHunt::CaseInt(-2500, -4000, 4000);
	const int32 Y = BugHunt::CaseInt(-2500, -4000, 4000);
	const int32 Yaw = BugHunt::CaseInt(45, 0, 359);
	BugHunt::Note(BugHunt::Fmt<Lab21Text::Scene>(X, Y, Yaw));

	const FVector Feet(static_cast<float>(X), static_cast<float>(Y), 1000.f);
	ABH_Miner* Miner = SpawnLabActor<ABH_Miner>(Feet);
	Miner->SetActorRotation(FRotator(0.f, static_cast<float>(Yaw), 0.f));
	ABH_Lantern* Lantern = SpawnLabActor<ABH_Lantern>(Feet + FVector(100.f, 0.f, 0.f));

	// 뼈 위치가 계산되도록 한 프레임 기다린 뒤 든다.
	// [JA] ボーンの位置が計算されるよう 1 フレーム待ってから持つ。
	After(0.f, [this, Miner, Lantern, Feet]
	{
		Miner->HoldLantern(Lantern);
		After(0.2f, [this, Miner, Lantern, Feet]
		{
			const FVector At = Lantern->GetActorLocation();
			BugHunt::ExpectTrue(BugHunt::Fmt<Lab21Text::Raised>(At.Z - Feet.Z), At.Z - Feet.Z > 80.f);
			const float Off = FVector::Dist(At, Miner->Mesh->GetBoneLocation(TEXT("hand_r")));
			BugHunt::ExpectTrue(BugHunt::Fmt<Lab21Text::InHand>(Off), Off < 5.f);
			FinishLab();
		});
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 붙이기는 됐는데(광부를 옮기면 랜턴도 따라옵니다) 랜턴이 손이 아니라 발밑(메시의 원점)에 있습니다. 경고도 에러도 없습니다.
// HoldLantern 에서 멈추고 조사식에 Mesh->DoesSocketExist(LanternSocket) 를 넣어 보세요.
// 이 메시에는 어떤 소켓과 뼈가 있을까요? 스켈레탈 메시 에디터의 스켈레톤 트리나 Mesh->GetAllSocketNames() 로 확인하세요.

// [JA] 取り付けはできた（鉱夫を動かすとランタンもついてくる）のに、ランタンが手ではなく足元（メッシュの原点）にあります。警告もエラーもありません。
// [JA] HoldLantern で止めて、ウォッチに Mesh->DoesSocketExist(LanternSocket) を入れてみてください。
// [JA] このメッシュにはどんなソケットとボーンがあるのでしょう？ スケルタルメッシュ エディターのスケルトンツリーや Mesh->GetAllSocketNames() で確認してください。
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 없는 소켓 이름으로 붙이거나 GetSocketLocation/GetSocketTransform 을 부르면 엔진은 조용히 컴포넌트 자체의 트랜스폼을 씁니다.
// 스켈레탈 메시의 원점은 보통 발 사이 바닥이라서 "발밑에 붙은 무기"가 됩니다. 소켓 이름은 메시(스켈레톤)마다 다릅니다.
// 뼈 이름도 소켓 이름 자리에 쓸 수 있습니다(FName 비교라 대소문자는 구분하지 않지만 철자는 맞아야 합니다).

// [JA] 存在しないソケット名で付けたり GetSocketLocation/GetSocketTransform を呼んだりすると、エンジンは黙ってコンポーネント自体のトランスフォームを使います。
// [JA] スケルタルメッシュの原点はふつう両足の間の床なので、「足元に付いた武器」になります。ソケット名はメッシュ（スケルトン）ごとに違います。
// [JA] ボーン名もソケット名の代わりに使えます（FName の比較なので大文字・小文字は区別しませんが、つづりは合っていなければなりません）。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 이 메시에 있는 이름을 씁니다. 오른손 뼈는 hand_r 입니다.
//     FName LanternSocket = TEXT("hand_r");
// 실제 게임에서는 스켈레톤에 손잡이 위치에 맞춘 소켓(소켓 매니저에서 추가)을 만들어 그 이름을 씁니다.
// 이름을 코드에 적을 때는 ensure(Mesh->DoesSocketExist(Name)) 같은 확인을 붙여 두면 조용한 실패를 막을 수 있습니다.

// [JA] このメッシュにある名前を使います。右手のボーンは hand_r です。
// [JA]     FName LanternSocket = TEXT("hand_r");
// [JA] 実際のゲームでは、スケルトンに持ち手の位置に合わせたソケット（ソケットマネージャーで追加）を作って、その名前を使います。
// [JA] 名前をコードに書くときは ensure(Mesh->DoesSocketExist(Name)) のような確認を付けておくと、静かな失敗を防げます。
#pragma endregion
