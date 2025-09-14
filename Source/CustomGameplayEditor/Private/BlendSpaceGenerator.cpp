#if WITH_EDITOR
#include "BlendSpaceGenerator.h"

#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/BlendSpaceFactoryNew.h"
#include "KillingFloorLike/BaseWeapon.h"
#include "KillingFloorLike/KFLikeGameInstance.h"
#include "UObject/SavePackage.h"

class IAssetTools;
class FAssetToolsModule;
class FContentBrowserModule;

UBlendSpaceGenerator::UBlendSpaceGenerator()
{
}


// ... SetWeaponData() 함수는 이전 제안대로 유지 ...
void UBlendSpaceGenerator::SetWeaponData()
{
	// 런타임에서 데이터 테이블 로드
	const FString DataTablePath = TEXT("DataTable'/Game/Data/_weapon._weapon'");
	WeaponDataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);

	if (WeaponDataTable)
	{
		UE_LOG(LogTemp, Display, TEXT("Success Data : %s"), *DataTablePath);

		// 무기 데이터 로드 및 맵 초기화
		for (const auto& RowPair : WeaponDataTable->GetRowMap())
		{
			FWeaponData* Row = reinterpret_cast<FWeaponData*>(RowPair.Value);
			if (Row)
			{
				// Emplace를 사용하여 더 효율적으로 데이터를 추가합니다.
				WeaponDataMap.Emplace(Row->id, *Row);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load data table: %s"), *DataTablePath);
	}
}

void UBlendSpaceGenerator::GenerateBlendSpace2D()
{
	// 2. 무기 데이터 초기화
	if (WeaponDataMap.IsEmpty())
	{
		SetWeaponData();
	}

	// 3. 에셋 레지스트리에서 애니메이션 시퀀스 로드
	TArray<FAssetData> AssetDataList = GetAnimationAssets(AssetPath);

	// 4. BlendSpace 생성 및 초기화 반복
	for (auto& [Key, Value] : WeaponDataMap)
	{
		TArray<FAssetData> SelectedAssets = FilterAssetsByAnimationName(AssetDataList, Value.animation_name);

		for (int i = 0; i < 2; i++)
		{
			const bool bIsSit = (i == 1);
			FString NewAssetName = bIsSit
				                       ? FString::Printf(TEXT("BS_%s_CH"), *Value.animation_name)
				                       : FString::Printf(TEXT("BS_%s"), *Value.animation_name);
			FString FullPackageName = PlayerBlendSpacePath / NewAssetName;

			UBlendSpace* BlendSpace = CreateOrFindBlendSpace(FullPackageName, NewAssetName);
			if (!BlendSpace)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to create or find BlendSpace: %s"), *FullPackageName);
				continue;
			}

			// BlendParameters 설정
			SetupBlendParameters(BlendSpace);

			// 샘플 추가
			AddSamplesToBlendSpace(BlendSpace, SelectedAssets, bIsSit);

			// BlendSpace 초기화 및 저장
			FinalizeAndSaveBlendSpace(BlendSpace, FullPackageName);
		}
	}
}

// ---------------------------
// 각 기능별 분리 함수 예시

TArray<FAssetData> UBlendSpaceGenerator::GetAnimationAssets(const FString& Path)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");
	FARFilter Filter;
	Filter.PackagePaths.Add(*Path);
	Filter.ClassPaths.Add(UAnimSequence::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetDataList);
	return AssetDataList;
}

TArray<FAssetData> UBlendSpaceGenerator::FilterAssetsByAnimationName(const TArray<FAssetData>& Assets,
                                                                     const FString& AnimationName)
{
	return Assets.FilterByPredicate([AnimationName](const FAssetData& AssetData)
	{
		FString Name = AssetData.AssetName.ToString();
		TArray<FString> Tokens;
		Name.ParseIntoArray(Tokens, TEXT("_"));
		return AnimationName.Equals(Tokens.Last());
	});
}

UBlendSpace* UBlendSpaceGenerator::CreateOrFindBlendSpace(const FString& PackageName, const FString& AssetName)
{
	UPackage* Package = CreatePackage(*PackageName);
	if (!Package) return nullptr;

	UBlendSpace* BlendSpace = FindObject<UBlendSpace>(Package, *AssetName);
	if (!BlendSpace)
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UBlendSpaceFactoryNew* Factory = NewObject<UBlendSpaceFactoryNew>();
		// 1. Skeleton 로드
		USkeleton* TargetSkeleton = LoadObject<USkeleton>(
			nullptr,
			TEXT(
				"/Game/KF/Character/KF_Soldier_Trip/SkeletalMesh/British_Soldier1/SkeletalMeshes/British_Soldier1_Skeleton.British_Soldier1_Skeleton"));
		if (!TargetSkeleton)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to load Skeleton"));
			return nullptr;
		}
		Factory->TargetSkeleton = TargetSkeleton;

		BlendSpace = Cast<UBlendSpace>(
			AssetTools.CreateAsset(AssetName, PlayerBlendSpacePath, UBlendSpace::StaticClass(), Factory));
	}
	return BlendSpace;
}

void UBlendSpaceGenerator::SetupBlendParameters(UBlendSpace* BlendSpace)
{
	FProperty* Prop = BlendSpace->GetClass()->FindPropertyByName(FName("BlendParameters"));
	if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
	{
		UScriptStruct* StructType = StructProp->Struct;
		void* ArrayPtr = StructProp->ContainerPtrToValuePtr<void>(BlendSpace);

		// X축 (Angle)
		void* ElementPtr = (uint8*)ArrayPtr + 0 * StructType->GetStructureSize();
		FBlendParameter* Param = reinterpret_cast<FBlendParameter*>(ElementPtr);
		Param->DisplayName = "Angle";
		Param->Min = -180;
		Param->Max = 180;
		Param->GridNum = 4;
		Param->bSnapToGrid = true;

		// Y축 (Speed)
		ElementPtr = (uint8*)ArrayPtr + 1 * StructType->GetStructureSize();
		Param = reinterpret_cast<FBlendParameter*>(ElementPtr);
		Param->DisplayName = "Speed";
		Param->Min = -100;
		Param->Max = 100;
		Param->GridNum = 4;
		Param->bSnapToGrid = true;
	}
}

void UBlendSpaceGenerator::AddSamplesToBlendSpace(UBlendSpace* BlendSpace, const TArray<FAssetData>& Assets,
                                                  bool bIsCharacter)
{
	for (const FAssetData& AssetData : Assets)
	{
		UAnimSequence* Anim = Cast<UAnimSequence>(AssetData.GetAsset());
		if (!Anim) continue;

		FString Name = AssetData.AssetName.ToString();
		TArray<FString> Tokens;
		Name.ParseIntoArray(Tokens, TEXT("_"));
		if (Tokens.Num() <= 0) continue;

		FString Prefix = Tokens[0];
		if (!bIsCharacter)
		{
			AddSamplesForStandPrefix(BlendSpace, Prefix, Anim);
		}
		else
		{
			AddSamplesForSitPrefix(BlendSpace, Prefix, Anim);
		}
	}
}

void UBlendSpaceGenerator::AddSamplesForStandPrefix(UBlendSpace* BlendSpace, const FString& Prefix, UAnimSequence* Anim)
{
	if (Prefix == "Idle")
	{
		TArray<FVector> Positions = {FVector(0, 0, 0), FVector(-180, 0, 0), FVector(180, 0, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "JogB")
	{
		TArray<FVector> Positions = {FVector(0, -100, 0), FVector(180, -100, 0), FVector(-180, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "JogF")
	{
		TArray<FVector> Positions = {FVector(0, 100, 0), FVector(180, 100, 0), FVector(-180, 100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "JogL")
	{
		TArray<FVector> Positions = {FVector(-90, 100, 0), FVector(-90, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "JogR")
	{
		TArray<FVector> Positions = {FVector(90, 100, 0), FVector(90, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "TurnL")
	{
		BlendSpace->AddSample(Anim, FVector(-90, 0, 0));
	}
	else if (Prefix == "TurnR")
	{
		BlendSpace->AddSample(Anim, FVector(90, 0, 0));
	}
}

void UBlendSpaceGenerator::AddSamplesForSitPrefix(UBlendSpace* BlendSpace, const FString& Prefix,
                                                  UAnimSequence* Anim)
{
	if (Prefix == "CHIdle")
	{
		TArray<FVector> Positions = {FVector(0, 0, 0), FVector(-180, 0, 0), FVector(180, 0, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "CHWalkB")
	{
		TArray<FVector> Positions = {FVector(0, -100, 0), FVector(180, -100, 0), FVector(-180, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "CHWalkF")
	{
		TArray<FVector> Positions = {FVector(0, 100, 0), FVector(180, 100, 0), FVector(-180, 100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "CHWalkL")
	{
		TArray<FVector> Positions = {FVector(-90, 100, 0), FVector(-90, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
	else if (Prefix == "CHWalkR")
	{
		TArray<FVector> Positions = {FVector(90, 100, 0), FVector(90, -100, 0)};
		for (const FVector& Pos : Positions) BlendSpace->AddSample(Anim, Pos);
	}
}

void UBlendSpaceGenerator::FinalizeAndSaveBlendSpace(UBlendSpace* BlendSpace, const FString& FullPackageName)
{
	FAssetRegistryModule::AssetCreated(BlendSpace);

	//중요!
	BlendSpace->ResampleData();
	BlendSpace->ValidateSampleData();

	BlendSpace->Modify();
	BlendSpace->PostEditChange();
	BlendSpace->MarkPackageDirty();

	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		*FullPackageName, FPackageName::GetAssetPackageExtension());

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

	bool bSaved = UPackage::SavePackage(BlendSpace->GetOutermost(), BlendSpace, *PackageFileName, SaveArgs);
	if (!bSaved)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save BlendSpace package: %s"), *FullPackageName);
	}
}
#endif
