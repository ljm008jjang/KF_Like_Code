// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "WeaponAnimInstance.generated.h"

enum class EWeaponAnimationType : uint8;
class AKillingFloorLikeCharacter;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UWeaponAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	virtual void NativeInitializeAnimation() override;

private:
	UPROPERTY(BlueprintGetter = GetKFCharacter)
	AKillingFloorLikeCharacter* KFCharacter;

public:
	UFUNCTION(BlueprintGetter, BlueprintPure)
	AKillingFloorLikeCharacter* GetKFCharacter() const
	{
		return KFCharacter;
	}

	UFUNCTION(BlueprintPure, meta=(BlueprintThreadSafe))
	bool GetCharacterIsIron();
};
