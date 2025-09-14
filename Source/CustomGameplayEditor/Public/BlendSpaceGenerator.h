#pragma once

#include "CoreMinimal.h"
#include "KillingFloorLike/BaseWeapon.h"
#include "BlendSpaceGenerator.generated.h"

UCLASS()
class CUSTOMGAMEPLAYEDITOR_API UBlendSpaceGenerator : public UObject
{
	GENERATED_BODY()

	const FString AssetPath = "/Game/KF/Character/KF_Soldier_Trip/SkeletalMesh/British_Soldier1/SkeletalMeshes";

public:
	UBlendSpaceGenerator();

	void SetWeaponData();
	void GenerateBlendSpace2D();

	TArray<FAssetData> GetAnimationAssets(const FString& String);
	TArray<FAssetData> FilterAssetsByAnimationName(const TArray<FAssetData>& Assets, const FString& AnimationName);
	void AddSamplesToBlendSpace(UBlendSpace* BlendSpace, const TArray<FAssetData>& Assets, bool bIsCharacter);
	UBlendSpace* CreateOrFindBlendSpace(const FString& PackageName, const FString& AssetName);
	void SetupBlendParameters(UBlendSpace* BlendSpace);
	void AddSamplesForStandPrefix(UBlendSpace* BlendSpace, const FString& Prefix, UAnimSequence* Anim);
	void AddSamplesForSitPrefix(UBlendSpace* BlendSpace, const FString& Prefix, UAnimSequence* Anim);
	void FinalizeAndSaveBlendSpace(UBlendSpace* BlendSpace, const FString& FullPackageName);

private:
	UPROPERTY()
	UDataTable* WeaponDataTable;

	UPROPERTY()
	TMap<int32, FWeaponData> WeaponDataMap;
};
