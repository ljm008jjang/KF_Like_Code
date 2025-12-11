// Copyright Epic Games, Inc. All Rights Reserved.


#include "TP_WeaponComponent.h"
#include "KillingFloorLikeCharacter.h"
#include "KillingFloorLikeProjectile.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Kismet/GameplayStatics.h"
#include "EnhancedInputSubsystems.h"
#include "ObjectPoolManager.h"
#include "SoundManager.h"

// Sets default values for this component's properties
UTP_WeaponComponent::UTP_WeaponComponent()
{
	// Default offset from the character location for projectiles to spawn
	MuzzleOffset = FVector(100.0f, 0.0f, 10.0f);
}


void UTP_WeaponComponent::Fire()
{
	if (IsValid(Character) == false || IsValid(Character->GetController()) == false)
	{
		return;
	}

	// Try and fire a projectile
	if (IsValid(ProjectileClass))
	{
		UWorld* const World = GetWorld();
		if (IsValid(World))
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			const FRotator SpawnRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			const FVector SpawnLocation = GetOwner()->GetActorLocation() + SpawnRotation.RotateVector(MuzzleOffset);

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			ActorSpawnParams.Owner = GetOwner(); // 현재 액터의 Owner를 설정

			// Spawn the projectile at the muzzle
			AKillingFloorLikeProjectile* projectile = GetWorld()->GetGameInstance()->GetSubsystem<UObjectPoolManager>()
			                                                    ->GetFromPoolTemplate<
				                                                    AKillingFloorLikeProjectile>(
				                                                    GetWorld(), ProjectileClass, SpawnLocation,
				                                                    SpawnRotation,
				                                                    ActorSpawnParams);
			if (projectile)
			{
				//projectile->Initialize(GunDamage);
			}
		}
	}

	// Try and play the sound if specified
	if (IsValid(FireSound))
	{
		UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USoundManager>()->Multi_Play3DSound(
			FireSound, Character->GetActorLocation());
		//UGameplayStatics::PlaySoundAtLocation(this, FireSound, Character->GetActorLocation());
	}

	// Try and play a firing animation if specified
	if (IsValid(FireAnimation))
	{
		// Get the animation object for the arms mesh
		UAnimInstance* AnimInstance = Character->GetMesh1P()->GetAnimInstance();
		if (IsValid(AnimInstance))
		{
			AnimInstance->Montage_Play(FireAnimation, 1.f);
		}
	}
}

EWeaponType UTP_WeaponComponent::GetWeaponType()
{
	return WeaponType;
}

void UTP_WeaponComponent::AttachWeapon(AKillingFloorLikeCharacter* TargetCharacter)
{
	Character = TargetCharacter;
	if (IsValid(Character) == false)
	{
		return;
	}

	SetSimulatePhysics(false);

	// Attach the weapon to the First Person Character
	FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);
	if (Character->GetMesh1P())
	{
		AttachToComponent(Character->GetMesh1P(), AttachmentRules, FName(TEXT("GripPoint")));
	}

	// switch bHasRifle so the animation blueprint can switch to another animation set
	Character->SetHasRifle(true);
	//Character->PickUpWeapon(this);
}

void UTP_WeaponComponent::DropWeapon(AKillingFloorLikeCharacter* TargetCharacter)
{
	if (IsValid(Character) == false)
	{
		return;
	}
	Character = nullptr;
	DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	GetOwner()->SetActorLocation(GetOwner()->GetActorLocation() + TargetCharacter->GetActorForwardVector() * 300, true,
	                             nullptr, ETeleportType::TeleportPhysics);
	SetSimulatePhysics(true);
	AddImpulse(TargetCharacter->GetActorForwardVector() * 3000);
}

void UTP_WeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(Character) == false)
	{
		return;
	}

	if (APlayerController* PlayerController = Cast<APlayerController>(Character->GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->RemoveMappingContext(FireMappingContext);
		}
	}
}
