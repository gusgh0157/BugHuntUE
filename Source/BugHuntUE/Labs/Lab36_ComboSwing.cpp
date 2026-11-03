#include "Labs/Lab36_ComboSwing.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab36Text
{
BH_TEXT(AttackLog,  "  프레임 %llu: 공격 → 휘두르기 시작", "  フレーム %llu: 攻撃 → 振り開始");
BH_TEXT(EndLog,     "  프레임 %llu: 휘두르기 끝 → 콤보 +1", "  フレーム %llu: 振り終了 → コンボ +1");
BH_TEXT(Plan,       "휘두르기 %.2f초. 0초에 공격, %.1f초에 한 번 더 공격(연타)", "振り %.2f 秒。0 秒に攻撃、%.1f 秒にもう一度攻撃（連打）");
BH_TEXT(MidSwing,   "두 번째 휘두르기 도중: 공격 중(이동·회피 막힘)", "2 回目の振りの途中: 攻撃中（移動・回避不可）");
BH_TEXT(AfterSwing, "다 휘두른 뒤: 공격 중 아님", "振り終えた後: 攻撃中ではない");
BH_TEXT(Combo,      "끝까지 마친 휘두르기 수 (콤보)", "最後までやり終えた振りの数（コンボ）");
}

void ABH_Swordsman::BeginPlay()
{
	Super::BeginPlay();
	SwingMontage = BugHunt::MakeMontage(BugHunt::LoadTutorialAnim(TEXT("Tutorial_Walk_Fwd")));
	GetAnim()->OnMontageEnded.AddDynamic(this, &ABH_Swordsman::HandleMontageEnded);
}

void ABH_Swordsman::Attack()
{
	bAttacking = true;
	GetAnim()->Montage_Play(SwingMontage);
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab36Text::AttackLog>(GFrameCounter));
}

void ABH_Swordsman::HandleMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
	if (Montage != SwingMontage)
	{
		return;
	}
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab36Text::EndLog>(GFrameCounter));
	bAttacking = false;
	++CompletedSwings;
}

void ABH_Lab36::StartLab()
{
	ABH_Swordsman* Hero = SpawnLabActor<ABH_Swordsman>(FVector(300.f, 0.f, 100.f));
	const float Length = Hero->SwingMontage->GetPlayLength();
	const float SecondPress = 0.3f;
	BugHunt::Note(BugHunt::Fmt<Lab36Text::Plan>(Length, SecondPress));

	Hero->Attack();
	After(SecondPress, [Hero] { Hero->Attack(); });

	// 두 번째 휘두르기 도중
	// [JA] 2 回目の振りの途中
	After(SecondPress + 0.5f, [Hero]
	{
		BugHunt::ExpectTrue(BugHunt::Tr<Lab36Text::MidSwing>(), Hero->bAttacking);
	});

	// 두 번째 휘두르기가 끝난 뒤
	// [JA] 2 回目の振りが終わった後
	After(SecondPress + Length + 0.5f, [this, Hero]
	{
		BugHunt::ExpectTrue(BugHunt::Tr<Lab36Text::AfterSwing>(), !Hero->bAttacking);
		BugHunt::Expect(BugHunt::Tr<Lab36Text::Combo>(), 1, Hero->CompletedSwings);
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 공격 버튼은 두 번, "휘두르기 끝" 로그도 두 번입니다. 그런데 휘두르기를 끝까지 마친 건 몇 번인가요?
// 프레임 번호로 기록표를 만드세요: 공격(0초) / 공격(0.3초) / 휘두르기 끝 / 휘두르기 끝. 첫 번째 "끝"은 언제 찍혔나요?

// [JA] 攻撃ボタンは 2 回、「振り終了」のログも 2 回です。では、振りを最後までやり終えたのは何回ですか？
// [JA] フレーム番号で記録表を作ってください: 攻撃（0 秒）/ 攻撃（0.3 秒）/ 振り終了 / 振り終了。1 つ目の「終了」はいつ記録されましたか？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// HandleMontageEnded 에 중단점을 걸고 두 번 멈출 때마다 지역 창에서 bInterrupted 를 보세요.
// 호출 스택에서 첫 번째 호출이 어디서 왔는지도 보세요 (Montage_Play → 같은 그룹의 몽타주 정지 → ... 블렌드 아웃이 끝남).

// [JA] HandleMontageEnded にブレークポイントを置き、2 回止まるたびにローカル ウィンドウで bInterrupted を見てください。
// [JA] 呼び出し履歴で、1 回目の呼び出しがどこから来たかも見てください（Montage_Play → 同じグループのモンタージュ停止 → ... ブレンドアウトの終了）。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// OnMontageEnded 는 "끝났다"가 아니라 "이 몽타주 인스턴스가 사라졌다" 입니다. 끝까지 재생해도, 다른 몽타주에 밀려나도,
// 같은 몽타주를 다시 재생해 이전 것이 밀려나도, Montage_Stop 으로 멈춰도 불립니다. 이유는 bInterrupted 로 구분합니다.
// 밀려난 이전 휘두르기의 "끝"이 새 휘두르기의 상태(bAttacking)를 덮어쓰고, 콤보까지 올리고 있습니다.
// (블렌드 아웃이 시작될 때 오는 OnMontageBlendingOut 에도 같은 bInterrupted 가 있습니다.)

// [JA] OnMontageEnded は「終わった」ではなく「このモンタージュのインスタンスがなくなった」です。最後まで再生しても、別のモンタージュに押し出されても、
// [JA] 同じモンタージュを再生し直して前のものが押し出されても、Montage_Stop で止めても呼ばれます。理由は bInterrupted で区別します。
// [JA] 押し出された前の振りの「終了」が、新しい振りの状態（bAttacking）を上書きし、コンボまで増やしています。
// [JA] （ブレンドアウトが始まるときに来る OnMontageBlendingOut にも同じ bInterrupted があります。）
#pragma endregion
