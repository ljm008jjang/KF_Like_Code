// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KillingFloorLikeProjectile.h"
#include "HuskProjectile.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API AHuskProjectile : public AKillingFloorLikeProjectile
{
	GENERATED_BODY()
public:
	AHuskProjectile();


protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse,
	           const FHitResult& Hit) override;

	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
	                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
	                            bool bFromSweep, const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	float DamageRadius;
	UPROPERTY(EditAnywhere)
	float KnockbackStrength;
	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* NiagaraComp;
};
