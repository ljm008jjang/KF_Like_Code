// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "ConsumableWeapon.generated.h"

/**
 * 탄약, 개수가 있는 무기
 */
UCLASS()
class KILLINGFLOORLIKE_API AConsumableWeapon : public ABaseWeapon
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void SetLoadedAmmo(int32 NewLoadedAmmo)
	{
		if (HasAuthority())
		{
			LoadedAmmo = NewLoadedAmmo;
			OnRep_Ammo();
		}
	}

	UPROPERTY(ReplicatedUsing=OnRep_Ammo)
	int32 LoadedAmmo;
	UPROPERTY(ReplicatedUsing=OnRep_Ammo)
	int32 SavedAmmo;

	void SetSavedAmmo(int32 NewSavedAmmo)
	{
		if (HasAuthority())
		{
			SavedAmmo = NewSavedAmmo;
			OnRep_Ammo();
		}
	}

	UFUNCTION()
	void OnRep_Ammo();

	void ReloadAmmoAdd(int ReloadedAmmo);


public:
	bool IsReloadable();
	//virtual bool FireWeapon(bool IsSpecial) override;
	virtual bool CheckFireWeapon(bool IsSpecial) override;

	virtual bool IsAttackable() override;

	UFUNCTION(BlueprintCallable)
	virtual FText GetShopAmmoText();


	UFUNCTION(BlueprintCallable)
	virtual void AddAmmo(int AddAmmoAmount);

	UFUNCTION(BlueprintCallable)
	int32 GetAmmoFillCost();

	int32 GetLoadedAmmo() const
	{
		return LoadedAmmo;
	}

	int32 GetSavedAmmo() const
	{
		return SavedAmmo;
	}

	UFUNCTION(Server, Reliable)
	virtual void Server_BuyWeaponFill();

	virtual FText GetAmmoText() override;
	virtual FText GetClipText() override;

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ReloadAmmoOne();
	UFUNCTION(NetMulticast, Reliable)
	void Multi_ReloadAmmoOneCallback();
};
