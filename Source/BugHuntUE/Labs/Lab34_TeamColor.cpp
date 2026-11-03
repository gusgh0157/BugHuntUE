#include "Labs/Lab34_TeamColor.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"

namespace
{
// 기획서: 주황 팀 = #FF8000, 청록 팀 = #008080 (디자인 툴의 색상 선택기에서 고른 값)
const FColor OrangeTeam(0xFF, 0x80, 0x00);
const FColor TealTeam(0x00, 0x80, 0x80);

const FName ColorParam(TEXT("Color"));

// 머티리얼 파라미터용 색으로 바꾼다.
FLinearColor ToMaterialColor(const FColor& Color)
{
// [JA] 仕様書: オレンジチーム = #FF8000、ティールチーム = #008080（デザインツールのカラーピッカーで選んだ値）
	return FLinearColor(Color.R / 255.f, Color.G / 255.f, Color.B / 255.f, Color.A / 255.f);
}

// 화면에 나간 색을 디자인 툴 기준(sRGB 8비트)으로 다시 읽는다. 검사용.
FColor ReadBackAsDesignColor(const UMaterialInstanceDynamic* MID)
{
// [JA] マテリアル パラメーター用の色に変換する。
	FLinearColor Value;
	MID->GetVectorParameterValue(FHashedMaterialParameterInfo(ColorParam), Value);
	return Value.ToFColor(/*bSRGB=*/true);
}
}

// [JA] 画面に出た色を、デザインツール基準（sRGB 8 ビット）で読み直す。検査用。
void ABH_Lab34::StartLab()
{
	UMaterialInterface* Base = LoadObject<UMaterialInterface>(nullptr, TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	UnitMaterial = UMaterialInstanceDynamic::Create(Base, this);

	for (const FColor& Team : { OrangeTeam, TealTeam })
	{
		UnitMaterial->SetVectorParameterValue(ColorParam, ToMaterialColor(Team));
		const FColor Shown = ReadBackAsDesignColor(UnitMaterial);
		BugHunt::Note(FString::Printf(TEXT("기획서 #%02X%02X%02X → 화면 #%02X%02X%02X"),
			Team.R, Team.G, Team.B, Shown.R, Shown.G, Shown.B));
		BugHunt::Expect(FString::Printf(TEXT("#%02X%02X%02X 팀 색의 G 채널"), Team.R, Team.G, Team.B), Team.G, Shown.G);
		BugHunt::Expect(FString::Printf(TEXT("#%02X%02X%02X 팀 색의 B 채널"), Team.R, Team.G, Team.B), Team.B, Shown.B);
	}
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 0x00 과 0xFF 는 맞는데 0x80(128) 만 틀립니다. 틀린 값은 기획서보다 큰가요, 작은가요? (화면에서는 물 빠지고 밝게 보입니다)
// ToMaterialColor 에 중단점을 걸고 조사식에 Color.G / 255.f 와 FLinearColor(Color).G 를 나란히 넣어 보세요.
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// 디자인 툴, 텍스처의 "sRGB" 체크, FColor 는 감마가 적용된(sRGB) 값입니다. 사람 눈에 맞춰 어두운 쪽에 값을 더 촘촘히 씁니다.
// 머티리얼과 셰이더의 계산, FLinearColor 는 선형(Linear) 값입니다. 128/255 = 0.502 는 sRGB로는 "중간 밝기"지만
// 선형으로는 약 0.216 입니다.
#pragma endregion


// [JA] 0x00 と 0xFF は合っているのに、0x80（128）だけが違います。違う値は仕様書より大きいですか、小さいですか？（画面では色あせて明るく見えます）
// [JA] ToMaterialColor にブレークポイントを設定し、ウォッチに Color.G / 255.f と FLinearColor(Color).G を並べて入れてみてください。
#pragma region 힌트 3 / ヒント 3
// FColor → FLinearColor 변환에 감마 변환이 빠졌습니다. 엔진에는 이미 변환 함수가 있습니다:
//     FLinearColor(const FColor&) 생성자 / FLinearColor::FromSRGBColor  (sRGB → Linear)
//     FLinearColor::ToFColor(true) / ToFColorSRGB                       (Linear → sRGB)
// 반대로 FColor::ReinterpretAsLinear 는 변환 없이 /255 만 합니다. 이름 그대로 "재해석"입니다.
// 텍스처라면 같은 문제가 Texture 에셋의 sRGB 체크박스로 나타납니다 (색 텍스처는 켬, 노멀/마스크 텍스처는 끔).

// [JA] デザインツール、テクスチャの「sRGB」チェック、FColor はガンマが適用された（sRGB）値です。人の目に合わせて、暗い側に値をより細かく割り当てます。
// [JA] マテリアルとシェーダーの計算、FLinearColor はリニア（Linear）値です。128/255 = 0.502 は sRGB では「中間の明るさ」ですが、
// [JA] リニアでは約 0.216 です。
#pragma endregion

// [JA] FColor → FLinearColor の変換でガンマ変換が抜けています。エンジンにはすでに変換関数があります:
// [JA]     FLinearColor(const FColor&) コンストラクター / FLinearColor::FromSRGBColor  (sRGB → Linear)
// [JA]     FLinearColor::ToFColor(true) / ToFColorSRGB                       (Linear → sRGB)
// [JA] 逆に FColor::ReinterpretAsLinear は変換せずに /255 するだけです。名前のとおり「再解釈」です。
// [JA] テクスチャなら、同じ問題が Texture アセットの sRGB チェックボックスとして現れます（カラーテクスチャはオン、ノーマル／マスクテクスチャはオフ）。
