// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ConsumableWeapon.h"
#include "GrenadeWeapon.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API AGrenadeWeapon : public AConsumableWeapon
{
	GENERATED_BODY()

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual bool CheckFireWeapon(bool IsSpecial) override;

	UFUNCTION(Server,Reliable)
	void Server_SetDelayBoom();
	void Boom();
	FText GetAmmoText();

	virtual void AddAmmo(int AddAmmoAmount) override;
	virtual void Server_BuyWeaponFill_Implementation() override;
	virtual FText GetShopAmmoText() override;

private:
	FTimerHandle BoomTimerHandle;
};
