// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "WeaponSkillInterface.h"
#include "MeleeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API AMeleeWeapon : public ABaseWeapon, public IWeaponSkillInterface
{
	GENERATED_BODY()

public:
	//virtual bool FireWeapon(bool IsSpecial) override;

private:
	UPROPERTY(EditAnywhere)
	float AttackRange = 100;

	/*UFUNCTION(BlueprintCallable)
	virtual void SkillFire(float SkillAttackDamage) override;*/
};
