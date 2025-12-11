// Fill out your copyright notice in the Description page of Project Settings.


#include "ResourceManager.h"
#include "KFLikeGameInstance.h"
#include "NiagaraSystem.h"

UObject* UResourceManager::LoadAssetByClass(const FString& AssetPath, UClass* AssetClass)
{
	if (CachedResources.Contains(AssetPath))
	{
		UObject* CachedAsset = CachedResources[AssetPath].Get();
		if (IsValid(CachedAsset) == false)
		{
			CachedResources.Remove(AssetPath);
			UE_LOG(LogTemp, Warning, TEXT("Removed invalid cached asset: %s"), *AssetPath);
		}
		else
		{
			return CachedAsset;
		}
	}


	//UObject* LoadedAsset = StaticLoadObject(AssetClass, nullptr, *AssetPath);
	TSoftObjectPtr<UObject> SoftAsset(StaticLoadObject(AssetClass, nullptr, *AssetPath));

	if (SoftAsset.IsValid())
	{
		CachedResources.Add(AssetPath, SoftAsset);
		return SoftAsset.Get();
	}

	UE_LOG(LogTemp, Error, TEXT("Failed to load asset: %s"), *AssetPath);
	return nullptr;
}

UTexture2D* UResourceManager::LoadTexture(const FString& AssetPath)
{
	return Cast<UTexture2D>(LoadAssetByClass(AssetPath, UTexture2D::StaticClass()));
}

USoundBase* UResourceManager::LoadSound(const FString& AssetPath)
{
	UObject* Asset = LoadAssetByClass(AssetPath, USoundBase::StaticClass());
	if (!IsValid(Asset))
	{
		UE_LOG(LogTemp, Error, TEXT("Asset is invalid or pending kill. AssetPath: %s"), *AssetPath);
		return nullptr;
	}
	USoundBase* Sound = Cast<USoundBase>(Asset);
	if (IsValid(Sound) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast asset to USoundBase. AssetPath: %s"), *AssetPath);
	}
	return Sound;
}

UNiagaraSystem* UResourceManager::LoadEffect(const FString& AssetPath)
{
	UObject* Asset = LoadAssetByClass(AssetPath, UNiagaraSystem::StaticClass());
	if (!IsValid(Asset))
	{
		UE_LOG(LogTemp, Error, TEXT("Asset is invalid or pending kill. AssetPath: %s"), *AssetPath);
		return nullptr;
	}
	UNiagaraSystem* NiagaraSystem = Cast<UNiagaraSystem>(Asset);
	if (IsValid(NiagaraSystem) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast asset to UNiagaraSystem. AssetPath: %s"), *AssetPath);
	}
	return NiagaraSystem;
}

UTexture2D* UResourceManager::LoadPerkIconTexture(const EPerkType PerkType)
{
	return LoadTexture(GetKFGameInstance()->GetPerkImagePath(PerkType, 0));
}

UTexture2D* UResourceManager::LoadWeaponImageTexture(const int32 WeaponId)
{
	return LoadTexture(GetKFGameInstance()->GetWeaponImagePath(WeaponId));
}

UBlendSpace* UResourceManager::LoadPlayerBlendSpace(int32 WeaponId, bool IsSit)
{
	return Cast<UBlendSpace>(LoadAssetByClass(GetKFGameInstance()->GetPlayerBlendSpacePath(WeaponId, IsSit),
	                                          UBlendSpace::StaticClass()));
}

UAnimMontage* UResourceManager::LoadPlayerAnimMontage(int32 WeaponId)
{
	return Cast<UAnimMontage>(LoadAssetByClass(
		GetKFGameInstance()->GetPlayerAnimMontagePath(WeaponId, EWeaponAnimationType::Fire),
		UAnimMontage::StaticClass()));
}
