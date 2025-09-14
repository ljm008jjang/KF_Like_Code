// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConsumableWeapon.h"
#include "RangeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API ARangeWeapon : public AConsumableWeapon
{
	GENERATED_BODY()

protected:
	

public:


	UFUNCTION(BlueprintCallable)
	void Reload();







private:




	/*UPROPERTY(EditAnywhere, BlueprintGetter=GetMaxLoadedAmmon, Category="Stat")
	int32 MaxLoadedAmmo;
	UPROPERTY(EditAnywhere, Category="Stat")
	int32 MaxSavedAmmo;*/
};
