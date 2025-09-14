#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "UObject/Object.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"

#include "MyBlendSpaceGenerator.generated.h"  // 반드시 여기에 위치해야 함

UCLASS()
class CUSTOMGAMEPLAYEDITOR_API UMyBlendSpaceGenerator : public UObject
{
	GENERATED_BODY()
public:
    /*static void GenerateBlendSpaces(
        USkeleton* TargetSkeleton,
        const FString& SavePath,
        const TMap<FString, TTuple<UAnimSequence*, UAnimSequence*, UAnimSequence*>>& WeaponAnimations);*/

	//UFUNCTION(Exec)
	void CreateBlendSpace2DAsset();
	
	//UFUNCTION(Exec)
    void GenerateBlendSpace2D(const FString& AssetName, const FString& PackagePath);
};