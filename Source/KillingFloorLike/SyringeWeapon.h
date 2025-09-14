// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConsumableWeapon.h"
#include "WeaponShootingInterface.h"
#include "WeaponSkillInterface.h"
#include "SyringeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API ASyringeWeapon : public AConsumableWeapon, public IWeaponSkillInterface
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY()
	FTimerHandle MyTimerHandle;
	
	int32 MaxMedicine = 100;
	int32 RegenRate = 7;
};
