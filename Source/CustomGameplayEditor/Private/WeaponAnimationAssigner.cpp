// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAnimationAssigner.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "KillingFloorLike/BaseWeapon.h"
#include "EditorAssetLibrary.h"

void UWeaponAnimationAssigner::AssignWeaponAnimations()
{
	SetWeaponData();
	SetBaseWeaponClasses();

	TArray<FAssetData> AssetDataList = GetWeaponMontage(
		"/Game/KF/Character/KF_Soldier_Trip/SkeletalMesh/British_Soldier1/SkeletalMeshes");
	for (auto& DataList : AssetDataList)
	{
		FString AssetName = DataList.AssetName.ToString();
		TArray<FString> ResultArray; // 결과를 담을 배열

		// MyString을 '_' 기준으로 나눠서 ResultArray에 저장합니다.
		AssetName.ParseIntoArray(ResultArray, TEXT("_"), true);
		UAnimMontage* Anim = Cast<UAnimMontage>(DataList.GetAsset());
		bool IsSit = false;
		if (AssetName.Contains("CH", ESearchCase::CaseSensitive))
		{
			IsSit = true;
		}
		EWeaponAnimationType AnimType = EWeaponAnimationType::None;
		if (AssetName.Contains("Attack", ESearchCase::CaseSensitive) || AssetName.Contains(
			"Fire", ESearchCase::CaseSensitive))
		{
			if (AssetName.Contains("Hard", ESearchCase::CaseSensitive))
			{
				AnimType = EWeaponAnimationType::SpecialFire;
			}
			else
			{
				AnimType = EWeaponAnimationType::Fire;
			}
		}
		else if (AssetName.Contains("Frag", ESearchCase::CaseSensitive))
		{
			AnimType = EWeaponAnimationType::Frag;
		}
		else if (AssetName.Contains("Reload", ESearchCase::CaseSensitive))
		{
			AnimType = EWeaponAnimationType::Reload;
		}

		if (ResultArray.IsValidIndex(ResultArray.Num() - 2) == false || BaseWeaponClassNameMap.Contains(
			ResultArray[ResultArray.Num() - 2]) == false || AnimType == EWeaponAnimationType::None)
		{
			//UE_LOG(LogTemp, Warning, TEXT("%s is Something wrong"), *AssetName);
			continue;
		}
		UClass* WeaponClass = BaseWeaponClassNameMap[ResultArray[ResultArray.Num() - 2]];
		if (!WeaponClass)
		{
			continue; // 클래스가 없으면 건너뜁니다.
		}

		// 2. 클래스에서 CDO를 가져와 수정합니다.
		ABaseWeapon* WeaponCDO = WeaponClass->GetDefaultObject<ABaseWeapon>();
		WeaponCDO->SetPlayerAnimation(Anim, AnimType, IsSit);
	}


	for (auto& Element : BaseWeaponClassNameMap)
	{
		// 3. 패키지를 더티로 마킹합니다. (클래스에서 가져와도 동일합니다)
		Element.Value->GetPackage()->MarkPackageDirty();

		// 4. [핵심] 클래스가 속한 '패키지'의 '이름'을 가져옵니다. 이것이 진짜 에셋 경로입니다.
		const FString AssetPath = Element.Value->GetPackage()->GetName();

		// 5. 올바른 에셋 경로로 저장합니다.
		if (UEditorAssetLibrary::SaveAsset(AssetPath))
		{
			UE_LOG(LogTemp, Log, TEXT("Successfully saved asset: %s"), *AssetPath);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save asset: %s"), *AssetPath);
		}
	}
}

void UWeaponAnimationAssigner::SetWeaponData()
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
				//WeaponDataMap.Emplace(Row->id, *Row);
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to load data table: %s"), *DataTablePath);
	}
}

void UWeaponAnimationAssigner::SetBaseWeaponClasses()
{
	BaseWeaponClassMap.Empty();

	TArray<FWeaponData*> AllRows;
	WeaponDataTable->GetAllRows(TEXT(""), AllRows);

	for (const auto& Row : WeaponDataTable->GetRowMap())
	{
		FName RowName = Row.Key;
		int32 NameAsInt = FCString::Atoi(*RowName.ToString());
		FWeaponData* WeaponData = (FWeaponData*)Row.Value;
		FString AssetName = "BP_" + (WeaponData->name.Replace(TEXT(" "), TEXT("")));
		FString AssetPath = "/Game/BluePrint/Weapon/" + AssetName + "." + AssetName + "_C";

		TSubclassOf<ABaseWeapon> LoadedClass = Cast<UClass>(
			StaticLoadObject(UClass::StaticClass(), nullptr, *AssetPath));


		if (LoadedClass)
		{
			LoadedClass->GetDefaultObject<ABaseWeapon>()->SetId(NameAsInt);
			LoadedClass->GetDefaultObject<ABaseWeapon>()->SetWeaponData(*WeaponData);
			LoadedClass->GetDefaultObject<ABaseWeapon>()->ClearPlayerAnimation();
			BaseWeaponClassMap.Add(NameAsInt, LoadedClass); // 배열에 추가
			BaseWeaponClassNameMap.Add(WeaponData->animation_name, LoadedClass);
		}
	}
}

TArray<FAssetData> UWeaponAnimationAssigner::GetWeaponMontage(const FString& Path)
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");

	FARFilter Filter;
	Filter.PackagePaths.Add(*Path);
	Filter.ClassPaths.Add(UAnimMontage::StaticClass()->GetClassPathName());
	Filter.bRecursivePaths = true;

	TArray<FAssetData> AssetDataList;
	AssetRegistryModule.Get().GetAssets(Filter, AssetDataList);
	return AssetDataList;
}
