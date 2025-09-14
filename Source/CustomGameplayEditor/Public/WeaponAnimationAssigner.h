// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KillingFloorLike/BaseWeapon.h"
#include "WeaponAnimationAssigner.generated.h"

/**
 * 
 */
UCLASS()
class CUSTOMGAMEPLAYEDITOR_API UWeaponAnimationAssigner : public UObject
{
	GENERATED_BODY()

private:
	UPROPERTY()
	UDataTable* WeaponDataTable;

	UPROPERTY()
	TMap<int32, TSubclassOf<class ABaseWeapon>> BaseWeaponClassMap;
	UPROPERTY()
	TMap<FString, TSubclassOf<class ABaseWeapon>> BaseWeaponClassNameMap;
	UPROPERTY()
	TMap<int32, FWeaponData> WeaponDataMap;

	void SetWeaponData();
	void SetBaseWeaponClasses();
	TArray<FAssetData> GetWeaponMontage(const FString& Path);

public:
	void AssignWeaponAnimations();
};
