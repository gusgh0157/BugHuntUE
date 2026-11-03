#include "Labs/Lab27_Knockback.h"
#include "Core/BugHuntCase.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/BoxComponent.h"
#include "Engine/CollisionProfile.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab27Text
{
BH_TEXT(KnockbackLog, "  넉백 %.0f cm → X=%.0f", "  ノックバック %.0f cm → X=%.0f");
BH_TEXT(Setup,        "거한 뒤 %.0f cm 에 성벽(두께 40cm). 망치로 쳐서 %.0f cm 밀어낸다.", "巨漢の後ろ %.0f cm に城壁（厚さ 40cm）。ハンマーで殴って %.0f cm 押し出す。");
BH_TEXT(NotThrough,   "거한이 성벽을 넘지 않았다 (거한 뒷면 X=%.0f ≤ 성벽 앞면 X=%.0f)", "巨漢が城壁を越えなかった（巨漢の背面 X=%.0f ≤ 城壁の前面 X=%.0f）");
BH_TEXT(ReachedWall,  "거한이 성벽에 닿을 때까지 밀렸다 (5cm 이내)", "巨漢が城壁に触れるまで押された（5cm 以内）");
}

ABH_Brute::ABH_Brute()
{
	Body = CreateDefaultSubobject<UBoxComponent>(TEXT("Body"));
	Body->InitBoxExtent(FVector(40.f, 40.f, 90.f));
	Body->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	RootComponent = Body;
}

void ABH_Brute::Knockback(const FVector& Direction, float Distance)
{
	const FVector Target = GetActorLocation() + Direction * Distance;
	SetActorLocation(Target);
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab27Text::KnockbackLog>(Distance, GetActorLocation().X));
}

ABH_CastleWall::ABH_CastleWall()
{
	Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Box"));
	Box->InitBoxExtent(FVector(20.f, 500.f, 300.f));
	Box->SetCollisionProfileName(UCollisionProfile::BlockAll_ProfileName);
	RootComponent = Box;
}

void ABH_Lab27::StartLab()
{
	// 기본 맵의 바닥과 플레이어에서 떨어진 빈 공중에서 실험한다.
	// [JA] デフォルトマップの床やプレイヤーから離れた、何もない空中で実験する。
	const FVector Base(0.f, 3000.f, 2000.f);
	const float WallX = static_cast<float>(BugHunt::CaseInt(300, 200, 400));
	const float Distance = static_cast<float>(BugHunt::CaseInt(500, 450, 900));
	SpawnLabActor<ABH_CastleWall>(Base + FVector(WallX, 0.f, 90.f));
	ABH_Brute* Brute = SpawnLabActor<ABH_Brute>(Base + FVector(0.f, 0.f, 90.f));
	BugHunt::Note(BugHunt::Fmt<Lab27Text::Setup>(WallX, Distance));

	After(0.1f, [this, Brute, WallX, Distance]
	{
		Brute->Knockback(FVector(1.f, 0.f, 0.f), Distance);
		const float WallFront = WallX - 20.f;
		const float BruteBack = Brute->GetActorLocation().X + 40.f;
		BugHunt::ExpectTrue(BugHunt::Fmt<Lab27Text::NotThrough>(BruteBack, WallFront), BruteBack <= WallFront + 1.f);
		BugHunt::ExpectTrue(BugHunt::Tr<Lab27Text::ReachedWall>(), BruteBack >= WallFront - 5.f && BruteBack <= WallFront + 1.f);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 거한이 성벽 너머에 서 있습니다. 둘 다 BlockAll 인데도요. show Collision 을 켜고 넉백 직후 일시 정지해서 보세요.
// SetActorLocation 에 F12 로 들어가 매개변수 목록을 보세요. 생략한 매개변수의 기본값은 무엇인가요?

// [JA] 巨漢が城壁の向こう側に立っています。どちらも BlockAll なのにです。show Collision をオンにし、ノックバック直後に一時停止して見てください。
// [JA] SetActorLocation に F12 で入ってパラメーター一覧を見てください。省略したパラメーターの既定値は何ですか？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 충돌 설정(Block)은 "움직이는 방법"이 충돌을 검사할 때만 의미가 있습니다. SetActorLocation(위치) 은 기본으로 순간이동입니다:
// 목적지에 그냥 놓고, 가는 길에 무엇이 있었는지는 보지 않습니다. 물리 시뮬레이션 중이 아니므로 물리가 밀어내 주지도 않습니다.

// [JA] コリジョン設定（Block）は、「動かし方」がコリジョンを調べるときにだけ意味があります。SetActorLocation(位置) は既定ではテレポートです:
// [JA] 目的地にそのまま置くだけで、途中に何があったかは見ません。物理シミュレーション中でもないので、物理が押し戻してくれることもありません。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
//     FHitResult Hit;
//     SetActorLocation(Target, /*bSweep*/ true, &Hit);   // 가는 길을 쓸어서(스윕) 막히면 그 앞에서 멈춘다
//     if (Hit.bBlockingHit) { /* 벽에 부딪힘: 스턴, 소리 */ }
// AddActorWorldOffset, SetWorldLocation 도 같은 bSweep 매개변수가 있습니다. 캐릭터라면 LaunchCharacter 나 CharacterMovement 가
// 스윕을 대신 해 줍니다. 스윕은 루트 컴포넌트의 모양으로만 합니다(자식 컴포넌트의 충돌은 보지 않음).

// [JA]     FHitResult Hit;
// [JA]     SetActorLocation(Target, /*bSweep*/ true, &Hit);   // 途中を掃引（スイープ）して、遮られたらその手前で止まる
// [JA]     if (Hit.bBlockingHit) { /* 壁にぶつかった: スタン、サウンド */ }
// [JA] AddActorWorldOffset、SetWorldLocation にも同じ bSweep パラメーターがあります。キャラクターなら LaunchCharacter や CharacterMovement が
// [JA] スイープを代わりにやってくれます。スイープはルートコンポーネントの形状だけで行います（子コンポーネントのコリジョンは見ません）。
#pragma endregion
