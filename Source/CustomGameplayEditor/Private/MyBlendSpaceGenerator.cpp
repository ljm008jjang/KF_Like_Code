#if WITH_EDITOR
#include "MyBlendSpaceGenerator.h"
#include "Animation/BlendSpace.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "FileHelpers.h"
#include "Factories/BlendSpaceFactoryNew.h"

class UBlendSpaceFactoryNew;

void UMyBlendSpaceGenerator::CreateBlendSpace2DAsset()
{
	GenerateBlendSpace2D("Test", "/Game/KF/Character/KF_Soldier_Trip/SkeletalMesh/British_Soldier1/BlendSpace");
}

void UMyBlendSpaceGenerator::GenerateBlendSpace2D(const FString& AssetName, const FString& PackagePath)
{
	// 에셋 경로 설정
	FString FullPackageName = PackagePath + "/" + AssetName;
	UPackage* Package = CreatePackage(*FullPackageName);

	// 블렌드스페이스 팩토리 생성
	UBlendSpaceFactoryNew* Factory = NewObject<UBlendSpaceFactoryNew>();
	Factory->TargetSkeleton = LoadObject<USkeleton>(
		nullptr, TEXT("/Game/KF/Character/KF_Soldier_Trip/SkeletalMesh/British_Soldier1/SkeletalMeshes/British_Soldier1_Skeleton.British_Soldier1_Skeleton")); // 자신에게 맞는 경로로 변경

	// 블렌드스페이스 생성
	UObject* NewAsset = Factory->FactoryCreateNew(
		UBlendSpace::StaticClass(), Package, *AssetName, RF_Public | RF_Standalone, nullptr, GWarn);
	

	if (!NewAsset)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create BlendSpace."));
		return;
	}

	UBlendSpace* BlendSpace = Cast<UBlendSpace>(NewAsset);
	if (!BlendSpace)
	{
		return;
	}

	/*BlendSpace->Modify();  // 트랜잭션 기록 (Undo/Redo 지원용)
	
	FBlendParameter& AxisX = BlendSpace->(0);
	AxisX.DisplayName = TEXT("Speed");
	AxisX.Max = 600.0f;
	AxisX.Min = 0.0f;
	AxisX.GridNum = 5;

	// Y축(Axis) 설정
	FBlendParameter& AxisY = BlendSpace->getax
	AxisY.DisplayName = TEXT("Direction");
	AxisY.Max = 180.0f;
	AxisY.Min = -180.0f;
	AxisY.GridNum = 5;


	


	// 다시 설정
	BlendSpace->param*/
	//BlendSpace->PostEditChange();

	// 패키지 더럽혔음을 표시
	NewAsset->MarkPackageDirty();

	// 저장할 패키지 리스트 생성
	TArray<UPackage*> PackagesToSave;
	PackagesToSave.Add(Package);

	// 에디터에서 패키지를 저장 (SavePackages 함수에 맞게 호출)
	bool bSaved = UEditorLoadingAndSavingUtils::SavePackages(
		PackagesToSave,
		true
	);

	if (!bSaved)
	{ 
		UE_LOG(LogTemp, Warning, TEXT("Failed to save package: %s"), *Package->GetName());
	}

	// 애셋 레지스트리에 등록
	FAssetRegistryModule::AssetCreated(NewAsset);

	UE_LOG(LogTemp, Log, TEXT("BlendSpace2D created at: %s"), *FullPackageName);
}
#endif
