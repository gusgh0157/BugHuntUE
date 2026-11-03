#include "Labs/Lab33_ItemIcons.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include "Engine/StreamableManager.h"
#include "Engine/Texture2D.h"

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
	UAssetManager::GetStreamableManager().RequestAsyncLoad(Item.Icon.ToSoftObjectPath(),
		FStreamableDelegate::CreateWeakLambda(this, [this, Item]
		{
			IconImage->SetBrushFromTexture(Item.Icon.Get());
			UE_LOG(LogBugHunt, Log, TEXT("  프레임 %llu: '%s' 아이콘 로드 완료 → 칸에 넣음"), GFrameCounter, *Item.Name);
		}));
}

void ABH_Lab33::StartLab()
{
	FBH_ItemData Ruby;
	Ruby.Name = TEXT("루비 반지");
	Ruby.Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Engine/EngineResources/AICON-Red.AICON-Red")));

	FBH_ItemData PotionData;
	PotionData.Name = TEXT("회복 물약");
	// [JA] まだメモリにないアイコンは非同期で読み込み、読み込み終わったらマスに入れる。
	PotionData.Icon = TSoftObjectPtr<UTexture2D>(FSoftObjectPath(TEXT("/Engine/EngineResources/DefaultTexture.DefaultTexture")));

	BugHunt::Note(FString::Printf(TEXT("루비 반지 아이콘 %s, 회복 물약 아이콘 %s"),
		Ruby.Icon.IsValid() ? TEXT("메모리에 있음") : TEXT("메모리에 없음"),
		PotionData.Icon.IsValid() ? TEXT("메모리에 있음") : TEXT("메모리에 없음")));

	Slot = NewObject<UBH_ItemSlot>(this);
	Slot->Init();

	// 스크롤: 이 칸에 루비 반지가 보였다가, 바로 다음 프레임에 회복 물약으로 바뀐다 (칸 재사용).
	UE_LOG(LogBugHunt, Log, TEXT("  프레임 %llu: 칸 ← 루비 반지"), GFrameCounter);
	Slot->ShowItem(Ruby);
	After(0.0f, [this, PotionData]
	{
		UE_LOG(LogBugHunt, Log, TEXT("  프레임 %llu: 칸 ← 회복 물약"), GFrameCounter);
		Slot->ShowItem(PotionData);
	});

	After(1.0f, [this, PotionData]
	{
		const UObject* Shown = Slot->IconImage->GetBrush().GetResourceObject();
		BugHunt::Note(FString::Printf(TEXT("칸에 적힌 이름 '%s', 그려진 아이콘 %s"), *Slot->ShownName, *GetNameSafe(Shown)));
		BugHunt::ExpectTrue(TEXT("칸 이름이 회복 물약"), Slot->ShownName == PotionData.Name);
		BugHunt::ExpectTrue(FString::Printf(TEXT("칸 아이콘이 회복 물약 아이콘 (실제: %s)"), *GetNameSafe(Shown)),
			Shown != nullptr && Shown == PotionData.Icon.Get());
		FinishLab();
	});
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 / ヒント 1 (먼저 직접 추적해 보고 펼치세요 / まず自分で追跡してから開いてください)
// 이름은 맞는데 그림이 다른 아이템 것입니다. Output Log의 프레임 번호를 순서대로 적어 보세요.
// "칸 ← 루비 반지", "칸 ← 회복 물약", "아이콘 로드 완료" 는 어떤 순서로 찍혔나요?
#pragma endregion

#pragma region 힌트 2 / ヒント 2
// 람다 안의 SetBrushFromTexture 줄에 중단점을 거세요. 멈췄을 때 지역 창의 Item.Name 과 this->ShownName 을 비교하세요.
// 이 콜백은 "지금 이 칸이 보여 주는 아이템"을 위한 것인가요?
// 호출 스택에서 누가 이 콜백을 불렀는지도 보세요 (FStreamableHandle ... 비동기 로딩 완료).
#pragma endregion

#pragma region 힌트 3 / ヒント 3
// 비동기 콜백은 "요청한 때의 세상"을 기억하고 "완료된 때의 세상"에서 실행됩니다. 그 사이에 칸이 재사용될 수 있습니다.
// 콜백 안에서 이 칸이 아직 같은 아이템을 보여 주는지 확인하거나, 새 아이템을 넣을 때 이전 요청을 취소하세요.
// (요청이 돌려주는 TSharedPtr<FStreamableHandle> 을 멤버로 들고 있다가 CancelHandle)
// ヒント: 行き詰まったときに 1 つずつ開いてください。（Ctrl+M, Ctrl+O = すべて折りたたむ）
#pragma endregion

// [JA] 名前は合っているのに、絵が別のアイテムのものです。アウトプットログのフレーム番号を順番に書き出してみてください。
// [JA] 「マス ← ルビーの指輪」「マス ← 回復ポーション」「アイコンのロード完了」はどの順番で出力されましたか？

// [JA] ラムダの中の SetBrushFromTexture の行にブレークポイントを設定してください。止まったら、ローカル ウィンドウの Item.Name と this->ShownName を比べてください。
// [JA] このコールバックは「今このマスが表示しているアイテム」のためのものですか？
// [JA] 呼び出し履歴で誰がこのコールバックを呼んだかも見てください（FStreamableHandle ... 非同期ロード完了）。

// [JA] 非同期コールバックは「リクエストしたときの世界」を覚えていて、「完了したときの世界」で実行されます。その間にマスが再利用されることがあります。
// [JA] コールバックの中でこのマスがまだ同じアイテムを表示しているか確認するか、新しいアイテムを入れるときに前のリクエストをキャンセルしてください。
// [JA] （リクエストが返す TSharedPtr<FStreamableHandle> をメンバーとして持っておき、CancelHandle）
