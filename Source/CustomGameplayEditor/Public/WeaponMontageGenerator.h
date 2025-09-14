// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "KillingFloorLike/BaseWeapon.h"
#include "WeaponMontageGenerator.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMGAMEPLAYEDITOR_API UWeaponMontageGenerator : public UObject
{
	GENERATED_BODY()

	TArray<FAssetData> GetWeaponSequence(const FString& Path);
	UAnimMontage* CreateOrFindMontage(UAnimSequence* AnimSequence, const FString& PackageName,
	                                  const FString& AssetName);
	void SetupMontage(UAnimMontage* Montage);
	void FinalizeAndSaveMontage(UAnimMontage* AnimMontage, const FString& FullPackageName);

	void SetWeaponData(TMap<int32, FWeaponData> &WeaponDataMap);
	TArray<FAssetData> FilterAssetsByAnimationName(const TArray<FAssetData>& Assets,
	                                               const FString& AnimationName);

public:
	void GenerateWeaponMontage();
};

