// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MeleeShootingComponent.h"
#include "SyringeShootingComponent.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KILLINGFLOORLIKE_API USyringeShootingComponent : public UMeleeShootingComponent
{
	GENERATED_BODY()

public:
	virtual bool Fire(AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial) override;
};
