// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "MonsterBlendSpaceGenerator.generated.h"

/**
 * 
 */
UCLASS()

class CUSTOMGAMEPLAYEDITOR_API UMonsterBlendSpaceGenerator : public UObject
{
	GENERATED_BODY()

	const FString AssetPath = "/Game/KF/Character/KF_Freaks_Trip";

public:
	UMonsterBlendSpaceGenerator();

	void SetWeaponData();
	void GenerateBlendSpace2D();

	TArray<FAssetData> GetAnimationAssets(const FString& String);
	TArray<FAssetData> FilterAssetsByAnimationName(const TArray<FAssetData>& Assets, const FString& AnimationName);
	void AddSamplesToBlendSpace(UBlendSpace* BlendSpace, const TArray<FAssetData>& Assets, bool bIsHeadless);
	UBlendSpace* CreateOrFindBlendSpace(const FString& TargetMonsterAssetPath, const FString& AssetName, const FString& SkeletalAssetPath);
	void SetupBlendParameters(UBlendSpace* BlendSpace);
	void AddSamplesForBasicPrefix(UBlendSpace* BlendSpace, const FString& AssetName, UAnimSequence* Anim);
	void AddSamplesForHeadlessPrefix(UBlendSpace* BlendSpace, const FString& Prefix, UAnimSequence* Anim);
	void FinalizeAndSaveBlendSpace(UBlendSpace* BlendSpace, const FString& FullPackageName);

private:
};
