// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponShootingInterface.h"
#include "Components/SceneComponent.h"
#include "GrenadeShootingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KILLINGFLOORLIKE_API UGrenadeShootingComponent : public USceneComponent, public IWeaponShootingInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGrenadeShootingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	/** 스폰할 수류탄 발사체의 클래스입니다. 에디터에서 설정해야 합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Config")
	TSubclassOf<class AGrenadeWeapon> GrenadeClass;
public:
	virtual bool Fire(AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial) override;

		
};
