// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponShootingInterface.generated.h"

struct FWeaponData;
// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UWeaponShootingInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class KILLINGFLOORLIKE_API IWeaponShootingInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual bool Fire(class AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial) = 0;

	// virtual bool IsAttackable() = 0;
};
