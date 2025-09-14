// Fill out your copyright notice in the Description page of Project Settings.
#if WITH_EDITOR

#include "WeaponMontageGenerator.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Factories/AnimMontageFactory.h"
#include "KillingFloorLike/KFLikeGameInstance.h"
#include "UObject/SavePackage.h"

void UWeaponMontageGenerator::GenerateWeaponMontage()
{
	TMap<int32, FWeaponData> WeaponDataMap;
	SetWeaponData(WeaponDataMap);

	TArray<FAssetData> AssetDataList = GetWeaponSequence(
		PlayerAnimMontagePath);

	for (auto& [Key, Value] : WeaponDataMap)
	{
		TArray<FAssetData> SelectedAssets = FilterAssetsByAnimationName(AssetDataList, Value.animation_name);

		for (auto& AssetData : SelectedAssets)
		{
			FString AssetName = AssetData.AssetName.ToString();
			if ((AssetName.Contains("Attack", ESearchCase::CaseSensitive) || AssetName.Contains(
					"Fire", ESearchCase::CaseSensitive) || AssetName.Contains("Frag", ESearchCase::CaseSensitive) ||
				AssetName.Contains("Hit", ESearchCase::CaseSensitive) || AssetName.Contains("Reload", ESearchCase::CaseSensitive)) == false)
			{
				continue;
			}

			UAnimSequence* AnimSequence = Cast<UAnimSequence>(AssetData.GetAsset());
			FString NewAssetName = FString::Printf(TEXT("%s_Montage"), *AssetName);
			FString FullPackageName = PlayerAnimMontagePath / NewAssetName;

			UAnimMontage* AnimMontage = CreateOrFindMontage(AnimSequence, FullPackageName, NewAssetName);
			if (!AnimMontage)
			{
				UE_LOG(LogTemp, Warning, TEXT("Failed to create or find AnimMontage: %s"), *FullPackageName);
				continue;
			}

			SetupMontage(AnimMontage);

			FinalizeAndSaveMontage(AnimMontage, FullPackageName);
		}
	}
}

TArray<FAssetData> UWeaponMontageGenerator::FilterAssetsByAnimationName(const TArray<FAssetData>& Assets,
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


void UWeaponMontageGenerator::SetWeaponData(TMap<int32, FWeaponData>& WeaponDataMap)
{
	// 런타임에서 데이터 테이블 로드
	const FString DataTablePath = TEXT("DataTable'/Game/Data/_weapon._weapon'");
	UDataTable* WeaponDataTable = LoadObject<UDataTable>(nullptr, *DataTablePath);
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

TArray<FAssetData> UWeaponMontageGenerator::GetWeaponSequence(const FString& Path)
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

UAnimMontage* UWeaponMontageGenerator::CreateOrFindMontage(UAnimSequence* AnimSequence, const FString& PackageName,
                                                           const FString& AssetName)
{
	UPackage* Package = CreatePackage(*PackageName);

	if (!Package) return nullptr;

	UAnimMontage* Montage = FindObject<UAnimMontage>(Package, *AssetName);
	if (!Montage)
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		UAnimMontageFactory* Factory = NewObject<UAnimMontageFactory>();

		USkeleton* TargetSkeleton = LoadObject<USkeleton>(
			nullptr,
			TEXT(
				"/Game/KF/Character/KF_Soldier_Trip/SkeletalMesh/British_Soldier1/SkeletalMeshes/British_Soldier1_Skeleton.British_Soldier1_Skeleton"));

		Factory->TargetSkeleton = TargetSkeleton;
		Factory->SourceAnimation = AnimSequence;

		Montage = Cast<UAnimMontage>(
			AssetTools.CreateAsset(AssetName, PlayerBlendSpacePath, UAnimMontage::StaticClass(), Factory));
	}
	return Montage;
}

void UWeaponMontageGenerator::SetupMontage(UAnimMontage* Montage)
{
	// 1. 유효성 검사
	if (Montage == nullptr)
	{
		return;
	}

	// 이미 'UpperBody' 슬롯을 사용하고 있다면 작업을 중단합니다.
	if (Montage->IsValidSlot(FName("UpperBody")))
	{
		return;
	}
	const FScopedTransaction Transaction(FText::FromString(TEXT("Set Montage Slot to UpperBody")));

	if (Montage->SlotAnimTracks.Num() > 0)
	{
		Montage->SlotAnimTracks[0].SlotName = FName("UpperBody");
	}
	else
	{
		// 수정할 트랙이 없는 경우 로그를 남깁니다.
		UE_LOG(LogTemp, Warning, TEXT("Montage '%s' has no slot tracks to modify."), *Montage->GetName());
	}
}

void UWeaponMontageGenerator::FinalizeAndSaveMontage(UAnimMontage* AnimMontage, const FString& FullPackageName)
{
	FAssetRegistryModule::AssetCreated(AnimMontage);
	// 3. 몽타주 애셋을 수정 가능한 상태로 만듭니다. (이것이 없으면 저장되지 않음)
	AnimMontage->Modify();
	AnimMontage->PostEditChange();
	AnimMontage->MarkPackageDirty();

	FString PackageFileName = FPackageName::LongPackageNameToFilename(
		*FullPackageName, FPackageName::GetAssetPackageExtension());

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone;

	bool bSaved = UPackage::SavePackage(AnimMontage->GetOutermost(), AnimMontage, *PackageFileName, SaveArgs);
	if (!bSaved)
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to save BlendSpace package: %s"), *FullPackageName);
	}
}
#endif
