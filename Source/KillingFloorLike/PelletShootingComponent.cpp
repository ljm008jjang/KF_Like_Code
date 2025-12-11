// Fill out your copyright notice in the Description page of Project Settings.


#include "PelletShootingComponent.h"

#include "DebugManagerComponent.h"
#include "KillingFloorLikeCharacter.h"
#include "KillingFloorLikeProjectile.h"
#include "ObjectPoolManager.h"
#include "Kismet/GameplayStatics.h"

bool UPelletShootingComponent::Fire(AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial)
{
	if (IsValid(ProjectileClass))
	{
		UWorld* const World = GetWorld();
		if (IsValid(World))
		{
			APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
			FRotator SpawnRotation; // = PlayerController->PlayerCameraManager->GetCameraRotation();
			// MuzzleOffset is in camera space, so transform it to world space before offsetting from the character location to find the final muzzle position
			FVector SpawnLocation;
			// = Character->GetController()->GetPlayerViewPoint() + SpawnRotation.RotateVector(MuzzleOffset);

			PlayerController->GetPlayerViewPoint(SpawnLocation, SpawnRotation);
			SpawnLocation += SpawnRotation.RotateVector(FVector(100, 0, 0));

			//Set Spawn Collision Handling Override
			FActorSpawnParameters ActorSpawnParams;
			ActorSpawnParams.SpawnCollisionHandlingOverride =
				ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
			ActorSpawnParams.Owner = GetOwner(); // 현재 액터의 Owner를 설정
			ActorSpawnParams.Instigator = Character->GetInstigator(); // 공격자를 설정 (옵션)

			// Spawn the projectile at the muzzle
			for (int i = 0; i < PelletCount; i++)
			{
				FRotator RealSpawnRotation = SpawnRotation;
				RealSpawnRotation.Yaw += FMath::RandRange(-SpreadAngle, SpreadAngle);
				RealSpawnRotation.Pitch += FMath::RandRange(-SpreadAngle, SpreadAngle);
				UDebugManagerComponent::DrawDebugLineManager(GetWorld(), SpawnLocation,
				                                             SpawnLocation + RealSpawnRotation.Vector() * 1000,
				                                             FColor::Red, false, 5);

				AKillingFloorLikeProjectile* projectile = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<
					UObjectPoolManager>()->GetFromPoolTemplate<
					AKillingFloorLikeProjectile>(GetWorld(), ProjectileClass, SpawnLocation, RealSpawnRotation,
					                             ActorSpawnParams);
				if (projectile)
				{
					projectile->Initialize(GetOwner(), Character->GetInstigator(),
					                       IsSpecial ? WeaponData->special_damage : WeaponData->damage,
					                       WeaponData->penetration, WeaponData->penetration_damage);
				}
			}
		}
	}

	return true;
}
