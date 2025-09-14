// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponAnimInstance.h"

#include "KillingFloorLikeCharacter.h"

void UWeaponAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();
	KFCharacter = Cast<AKillingFloorLikeCharacter>(GetOwningActor());
}

bool UWeaponAnimInstance::GetCharacterIsIron()
{
	if (KFCharacter == nullptr)
	{
		return false;
	}
	//UE_LOG(LogTemp, Warning, TEXT("GetIsIron : %d"), KFCharacter->GetIsIron());
	return KFCharacter->GetIsIron();
}

/*void UWeaponAnimInstance::Server_RequestShoot_Implementation()
{
	UAnimMontage* FireAnimMontage = GetMontage(EWeaponAnimationType::Fire);
	ABaseWeapon* CurrentWeapon = KFCharacter->GetCurrentWeapon();

	if (CurrentWeapon->IsAttackable() == false)
	{
		return;
	}

	if (Montage_IsPlaying(FireAnimMontage))
	{
		Montage_Stop(0, FireAnimMontage);
	}
	else if (Montage_IsPlaying(nullptr))
	{
		return;
	}

	Multicast_PlayAnimation(FireAnimMontage);
	CurrentWeapon->SetCurrentAttackCooltime();
}

UAnimMontage* UWeaponAnimInstance::GetMontage(EWeaponAnimationType AnimationType)
{
	ABaseWeapon* CurrentWeapon = KFCharacter->GetCurrentWeapon();
	int32 RandIdx = FMath::RandHelper(CurrentWeapon->GetAnimationMaxIndex(AnimationType));
	return CurrentWeapon->GetAnimation(AnimationType, IsIron, RandIdx);
}

void UWeaponAnimInstance::Multicast_PlayAnimation_Implementation(UAnimMontage* AnimMontage)
{
	Montage_Play(AnimMontage);
}*/
