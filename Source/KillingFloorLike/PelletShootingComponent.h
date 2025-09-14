// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ProjectileShootingComponent.h"
#include "PelletShootingComponent.generated.h"

/**
 * 
 */
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KILLINGFLOORLIKE_API UPelletShootingComponent : public UProjectileShootingComponent
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere)
	int32 PelletCount = 5;
	UPROPERTY(EditAnywhere)
	float SpreadAngle = 0.01f;
	
public:
	virtual bool Fire(AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial) override;
};
