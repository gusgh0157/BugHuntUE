#include "Labs/Lab33_ItemIcons.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"

// 실행 출력 문구 (한국어, 일본어) / 実行時の出力文言（韓国語、日本語）
namespace Lab33Text
{
BH_TEXT(IconLoaded,  "  프레임 %llu: '%s' 아이콘 로드 완료 → 칸에 넣음", "  フレーム %llu: 「%s」のアイコンのロード完了 → マスに入れた");
BH_TEXT(RubyRing,    "루비 반지", "ルビーの指輪");
BH_TEXT(Potion,      "회복 물약", "回復ポーション");
BH_TEXT(IconState,   "루비 반지 아이콘 %s, 회복 물약 아이콘 %s", "ルビーの指輪のアイコン %s、回復ポーションのアイコン %s");
BH_TEXT(InMemory,    "메모리에 있음", "メモリにある");
BH_TEXT(NotInMemory, "메모리에 없음", "メモリにない");
BH_TEXT(SlotRuby,    "  프레임 %llu: 칸 ← 루비 반지", "  フレーム %llu: マス ← ルビーの指輪");
BH_TEXT(SlotPotion,  "  프레임 %llu: 칸 ← 회복 물약", "  フレーム %llu: マス ← 回復ポーション");
BH_TEXT(ShownState,  "칸에 적힌 이름 '%s', 그려진 아이콘 %s", "マスに書かれた名前「%s」、描かれたアイコン %s");
BH_TEXT(NameIsPotion, "칸 이름이 회복 물약", "マスの名前が回復ポーション");
BH_TEXT(IconIsPotion, "칸 아이콘이 회복 물약 아이콘 (실제: %s)", "マスのアイコンが回復ポーションのアイコン（実際: %s）");
}

void UBH_ItemSlot::Init()
{
	IconImage = NewObject<UImage>(this, TEXT("IconImage"));
}

void UBH_ItemSlot::ShowItem(const FBH_ItemData& Item)
{
	ShownName = Item.Name;

	if (UTexture2D* Loaded = Item.Icon.Get())
	{
		IconImage->SetBrushFromTexture(Loaded);
		return;
	}

	// 아직 메모리에 없는 아이콘은 비동기로 불러서, 다 불러오면 칸에 넣는다.
	// [JA] まだメモリにないアイコンは非同期で読み込み、読み込み終わったらマスに入れる。
	UAssetManager::GetStreamableManager().RequestAsyncLoad(Item.Icon.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this, Item]
		{
			IconImage->SetBrushFromTexture(Item.Icon.Get());
			UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab33Text::IconLoaded>(GFrameCounter, *Item.Name));
		}));
}

void ABH_Lab33::StartLab()
{
	FBH_ItemData Ruby;
	Ruby.Name = BugHunt::Tr<Lab33Text::RubyRing>();
	Ruby.Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Engine/EngineResources/AICON-Red.AICON-Red")));

	FBH_ItemData PotionData;
	PotionData.Name = BugHunt::Tr<Lab33Text::Potion>();
	PotionData.Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture")));

	BugHunt::Note(BugHunt::Fmt<Lab33Text::IconState>(
		Ruby.Icon.IsValid() ? BugHunt::Tr<Lab33Text::InMemory>() : BugHunt::Tr<Lab33Text::NotInMemory>(),
		PotionData.Icon.IsValid() ? BugHunt::Tr<Lab33Text::InMemory>() : BugHunt::Tr<Lab33Text::NotInMemory>()));

	Slot = NewObject<UBH_ItemSlot>(this);
	Slot->Init();

	// 스크롤: 이 칸에 루비 반지가 보였다가, 바로 다음 프레임에 회복 물약으로 바뀐다 (칸 재사용).
	// [JA] スクロール: このマスにルビーの指輪が表示されてから、すぐ次のフレームで回復ポーションに変わる（マスの再利用）。
	UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab33Text::SlotRuby>(GFrameCounter));
	Slot->ShowItem(Ruby);
	After(0.0f, [this, PotionData]
	{
		UE_LOG(LogBugHunt, Log, TEXT("%s"), *BugHunt::Fmt<Lab33Text::SlotPotion>(GFrameCounter));
		Slot->ShowItem(PotionData);
	});

	After(1.0f, [this, PotionData]
	{
		const UObject* Shown = Slot->IconImage->GetBrush().GetResourceObject();
		BugHunt::Note(BugHunt::Fmt<Lab33Text::ShownState>(*Slot->ShownName, *GetNameSafe(Shown)));
		BugHunt::ExpectTrue(BugHunt::Tr<Lab33Text::NameIsPotion>(), Slot->ShownName == PotionData.Name);
		BugHunt::ExpectTrue(BugHunt::Fmt<Lab33Text::IconIsPotion>(*GetNameSafe(Shown)),
			Shown != nullptr && Shown == PotionData.Icon.Get());
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 이름은 맞는데 그림이 다른 아이템 것입니다. Output Log의 프레임 번호를 순서대로 적어 보세요.
// "칸 ← 루비 반지", "칸 ← 회복 물약", "아이콘 로드 완료" 는 어떤 순서로 찍혔나요?

// [JA] 名前は合っているのに、絵が別のアイテムのものです。アウトプットログのフレーム番号を順番に書き出してみてください。
// [JA] 「マス ← ルビーの指輪」「マス ← 回復ポーション」「アイコンのロード完了」はどの順番で出力されましたか？
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 람다 안의 SetBrushFromTexture 줄에 중단점을 거세요. 멈췄을 때 지역 창의 Item.Name 과 this->ShownName 을 비교하세요.
// 이 콜백은 "지금 이 칸이 보여 주는 아이템"을 위한 것인가요?
// 호출 스택에서 누가 이 콜백을 불렀는지도 보세요 (FStreamableHandle ... 비동기 로딩 완료).

// [JA] ラムダの中の SetBrushFromTexture の行にブレークポイントを設定してください。止まったら、ローカル ウィンドウの Item.Name と this->ShownName を比べてください。
// [JA] このコールバックは「今このマスが表示しているアイテム」のためのものですか？
// [JA] 呼び出し履歴で誰がこのコールバックを呼んだかも見てください（FStreamableHandle ... 非同期ロード完了）。
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 비동기 콜백은 "요청한 때의 세상"을 기억하고 "완료된 때의 세상"에서 실행됩니다. 그 사이에 칸이 재사용될 수 있습니다.
// 콜백 안에서 이 칸이 아직 같은 아이템을 보여 주는지 확인하거나, 새 아이템을 넣을 때 이전 요청을 취소하세요.
// (요청이 돌려주는 TSharedPtr<FStreamableHandle> 을 멤버로 들고 있다가 CancelHandle)

// [JA] 非同期コールバックは「リクエストしたときの世界」を覚えていて、「完了したときの世界」で実行されます。その間にマスが再利用されることがあります。
// [JA] コールバックの中でこのマスがまだ同じアイテムを表示しているか確認するか、新しいアイテムを入れるときに前のリクエストをキャンセルしてください。
// [JA] （リクエストが返す TSharedPtr<FStreamableHandle> をメンバーとして持っておき、CancelHandle）
#pragma endregion
