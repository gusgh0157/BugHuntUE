#include "Labs/Lab32_Highlight.h"
#include "Core/BugHuntReport.h"
#include "BugHuntUE.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "UObject/ConstructorHelpers.h"

const FName ABH_Crate::ColorParam(TEXT("Color"));

ABH_Crate::ABH_Crate()
{
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cube(TEXT("/Engine/BasicShapes/Cube.Cube"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial"));
	Mesh->SetStaticMesh(Cube.Object);
	Mesh->SetMaterial(0, Material.Object);
}

void ABH_Crate::SetHighlighted(bool bOn)
{
	UMaterialInstanceDynamic* MID = Cast<UMaterialInstanceDynamic>(Mesh->GetMaterial(0));
	if (!MID)
	{
		MID = UMaterialInstanceDynamic::Create(Mesh->GetMaterial(0), this);
	}
	MID->SetVectorParameterValue(ColorParam, bOn ? FLinearColor::Red : FLinearColor::White);
}

FLinearColor ABH_Crate::GetDisplayedColor() const
{
	FLinearColor Value = FLinearColor::Black;
	if (const UMaterialInterface* Material = Mesh->GetMaterial(0))
	{
		Material->GetVectorParameterValue(FHashedMaterialParameterInfo(ColorParam), Value);
	}
	return Value;
}

namespace
{
FString Describe(const FLinearColor& C)
{
	return FString::Printf(TEXT("(%.2f, %.2f, %.2f)"), C.R, C.G, C.B);
}
}

void ABH_Lab32::StartLab()
{
	ABH_Crate* Left = SpawnLabActor<ABH_Crate>(FVector(300.f, -150.f, 50.f));
	ABH_Crate* Right = SpawnLabActor<ABH_Crate>(FVector(300.f, 150.f, 50.f));
	const FLinearColor Original = Right->GetDisplayedColor();
	BugHunt::Note(FString::Printf(TEXT("두 상자의 원래 색 %s. 왼쪽 상자에 마우스를 올린다."), *Describe(Original)));

	Left->SetHighlighted(true);

	const FLinearColor LeftColor = Left->GetDisplayedColor();
	const FLinearColor RightColor = Right->GetDisplayedColor();
	BugHunt::Note(FString::Printf(TEXT("왼쪽 %s (머티리얼 %s), 오른쪽 %s (머티리얼 %s)"),
		*Describe(LeftColor), *GetNameSafe(Left->Mesh->GetMaterial(0)),
		*Describe(RightColor), *GetNameSafe(Right->Mesh->GetMaterial(0))));

	BugHunt::ExpectTrue(TEXT("왼쪽 상자가 빨갛게 강조됨"), LeftColor.Equals(FLinearColor::Red, 0.01f));
	BugHunt::ExpectTrue(TEXT("오른쪽 상자는 원래 색 그대로"), RightColor.Equals(Original, 0.01f));
	FinishLab();
}

// ============================================================================
// 힌트: 막힐 때 하나씩만 펼치세요. (Ctrl+M, Ctrl+O = 전부 접기)
// ============================================================================

#pragma region 힌트 1 (먼저 직접 추적해 보고 펼치세요)
// 크래시도 경고도 없습니다. SetVectorParameterValue 는 분명히 불렸는데 화면 색이 그대로입니다.
// SetHighlighted 에 중단점을 걸고, F10 으로 내려가며 조사식에 MID 와 Mesh->GetMaterial(0) 을 나란히 넣으세요.
// 두 주소(와 이름)가 같은가요? 로그의 "머티리얼 ..." 이름도 보세요.
#pragma endregion

#pragma region 힌트 2
// UMaterialInstanceDynamic::Create 는 "새 머티리얼 인스턴스를 만들기만" 합니다. 만든 걸 메시에 끼우는 일은 누가 하나요?
// 에디터에서 PIE 중 상자를 선택하고 Details 패널의 Materials 항목을 보면 지금 끼워진 머티리얼이 보입니다.
#pragma endregion

#pragma region 힌트 3
// 공유 머티리얼 에셋 자체를 바꾸면 그 머티리얼을 쓰는 모든 상자가 바뀝니다(그리고 에디터에선 에셋이 수정됨).
// 상자마다 따로 바꾸려면 상자마다 MID 를 만들어 "그 상자에" 끼워야 합니다.
// UPrimitiveComponent::CreateDynamicMaterialInstance(0) 은 만들고, 끼우고, 이미 MID 면 재사용까지 해 줍니다.
#pragma endregion
