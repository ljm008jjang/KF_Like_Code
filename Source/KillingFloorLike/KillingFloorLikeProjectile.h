// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "KillingFloorLikeProjectile.generated.h"

class UPoolableComponent;
class ABaseCharacter;
class USphereComponent;
class UProjectileMovementComponent;

UCLASS(config=Game)
class AKillingFloorLikeProjectile : public AActor
{
	GENERATED_BODY()


public:
	AKillingFloorLikeProjectile();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	/** Sphere collision component */
	UPROPERTY(VisibleDefaultsOnly, Category=Projectile)
	USphereComponent* CollisionComp;

	/** Projectile movement component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	UProjectileMovementComponent* ProjectileMovement;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPoolableComponent* PoolableComponent;

	UPROPERTY()
	FTimerHandle TimerHandle;
	
	UFUNCTION()
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
public:

	/** called when projectile hits something */
	/*UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	           const FHitResult& Hit);*/


	/** Returns CollisionComp subobject **/
	USphereComponent* GetCollisionComp() const { return CollisionComp; }
	/** Returns ProjectileMovement subobject **/
	UProjectileMovementComponent* GetProjectileMovement() const { return ProjectileMovement; }

	UFUNCTION(BlueprintCallable)
	void Initialize(AActor* NewOwner, APawn* NewInstigator, float GunDamage, int NewPenteration, float NewPenterationDamage);

	UFUNCTION(BlueprintGetter)
	float GetProjectileDamage();

	UFUNCTION(BlueprintGetter)
	int32 GetCurrentPenetrationCount();

	UFUNCTION(BlueprintCallable)
	void AddCurrentPenetrationCount(int32 AddPenetrationCount);

	UFUNCTION(BlueprintCallable)
	bool IsPenetrateOver();

	UFUNCTION(BlueprintCallable)
	void AddHittedActor(AActor* AddActor);

	UFUNCTION(BlueprintCallable)
	bool IsHittedActor(AActor* HitActor);

private:
	UPROPERTY(EditAnywhere, BlueprintGetter = GetProjectileDamage)
	float ProjectileDamage;

	UPROPERTY()
	int32 Penetration;
	UPROPERTY()
	float PenetrationDamageRatio;
	UPROPERTY(BlueprintGetter = GetCurrentPenetrationCount)
	int32 CurrentPenetrationCount = 0;

	UPROPERTY()
	TArray<AActor*> HittedActor;

	UFUNCTION()
	void OnPooledActivate();
	UFUNCTION(NetMulticast,Reliable)
	void OnPooledActivateCallback();
	UFUNCTION()
	void OnPooledReset();
};
