// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "WeaponShootingInterface.h"
#include "Components/SceneComponent.h"
#include "ProjectileShootingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KILLINGFLOORLIKE_API UProjectileShootingComponent : public USceneComponent, public IWeaponShootingInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UProjectileShootingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category=Projectile)
	TSubclassOf<class AKillingFloorLikeProjectile> ProjectileClass;
public:
	virtual bool Fire(AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial) override;
	
private:
	bool GetFireStartProperties(const AKillingFloorLikeCharacter* Character, FVector& OutLocation,
	                            FRotator& OutRotation) const;
	void SpawnAndInitializeProjectile(const FWeaponData* WeaponData, const FVector& SpawnLocation,
	                                  const FRotator& BaseRotation, const FActorSpawnParameters& SpawnParams,
	                                  float Damage,
	                                  UWorld* World) const;
};
