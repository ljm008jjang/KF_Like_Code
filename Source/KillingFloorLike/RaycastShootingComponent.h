// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponShootingInterface.h"
#include "Components/SceneComponent.h"
#include "RaycastShootingComponent.generated.h"


struct FWeaponData;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KILLINGFLOORLIKE_API URaycastShootingComponent : public USceneComponent, public IWeaponShootingInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	URaycastShootingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	virtual bool Fire(AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial) override;

private:
	TArray<AActor*> HittedActor;

	bool GetTraceStartEnd(const AKillingFloorLikeCharacter* Character, FVector& OutStart, FVector& OutEnd) const;
	void FilterAndPrioritizeHits(const TArray<FHitResult>& RawHits, TArray<FHitResult>& OutFinalTargets) const;
	void ApplyDamageWithPenetration(const TArray<FHitResult>& Targets, AKillingFloorLikeCharacter* Character, const FWeaponData* WeaponData, bool IsSpecial) const;
};
