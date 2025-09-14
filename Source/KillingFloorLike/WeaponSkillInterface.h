// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WeaponSkillInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable)
class UWeaponSkillInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class KILLINGFLOORLIKE_API IWeaponSkillInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// This function can be implemented in C++ (by overriding SpecialFire_Implementation) or in Blueprints.
	// Using BlueprintNativeEvent tells Unreal to generate a base (empty) implementation,
	// which resolves the "unresolved external symbol" linker error.
	// This allows classes to optionally implement this function.
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Weapon Skill")
	void SpecialFire();
};
