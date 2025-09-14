// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KFLikeGameInstance.h"
#include "UObject/NoExportTypes.h"
#include "ResourceManager.generated.h"

class UNiagaraSystem;
enum class EPerkType : uint8;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class KILLINGFLOORLIKE_API UResourceManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 싱글톤 접근자
	/*UFUNCTION(BlueprintCallable, Category="Global")
	static class UObjectPoolManager* GetInstance();*/

	UObject* LoadAssetByClass(const FString& AssetPath, UClass* AssetClass);
	UFUNCTION(BlueprintCallable, Category = "Resource")
	UTexture2D* LoadTexture(const FString& Path);
	UFUNCTION(BlueprintCallable, Category = "Resource")
	USoundBase* LoadSound(const FString& Path);
	UFUNCTION(BlueprintCallable, Category = "Resource")
	UNiagaraSystem* LoadEffect(const FString& Path);


	UFUNCTION(BlueprintCallable, Category = "Resource")
	UTexture2D* LoadPerkIconTexture(const EPerkType PerkType);
	UFUNCTION(BlueprintCallable, Category = "Resource")
	UTexture2D* LoadWeaponImageTexture(int32 WeaponId);

	UFUNCTION(BlueprintCallable, Category = "Resource")
	UBlendSpace* LoadPlayerBlendSpace(int32 WeaponId, bool IsSit);
	UFUNCTION(BlueprintCallable, Category = "Resource")
	UAnimMontage* LoadPlayerAnimMontage(int32 WeaponId);

private:
	UPROPERTY()
	TMap<FString, TSoftObjectPtr<UObject>> CachedResources;

	UPROPERTY()
	class UKFLikeGameInstance* KFLikeGameInstance; // 약한 참조 (소유하지 않음)
	UKFLikeGameInstance* GetKFGameInstance()
	{
		if (KFLikeGameInstance == nullptr)
		{
			KFLikeGameInstance = Cast<UKFLikeGameInstance>(GetGameInstance());
		}

		return KFLikeGameInstance;
	}
};
