// Fill out your copyright notice in the Description page of Project Settings.


#include "RangeWeapon.h"
#include "KillingFloorLikeCharacter.h"

class UPubSubManager;



void ARangeWeapon::Reload()
{
	if (HasAuthority() == false)
	{
		return;
	}
	if (SavedAmmo <= 0)
	{
		return;
	}
	int32 AllAmmonCount = LoadedAmmo + SavedAmmo;
	int32 ReloadAmmoCount = FMath::Min(GetMagazineCapacity(), AllAmmonCount);
	SetLoadedAmmo(ReloadAmmoCount);
	SetSavedAmmo(AllAmmonCount - ReloadAmmoCount);
}


