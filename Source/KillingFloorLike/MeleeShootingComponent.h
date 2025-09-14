// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WeaponShootingInterface.h"
#include "Components/SceneComponent.h"
#include "MeleeShootingComponent.generated.h"


class ABaseCharacter;

UENUM(BlueprintType)
enum class EWeaponSoundType : uint8
{
	None,
	Fire,
	Hit,
	Select,
	PutDown,
	Reload
};

USTRUCT(BlueprintType)
struct FWeaponSoundBases
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<USoundBase*> SoundBases;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KILLINGFLOORLIKE_API UMeleeShootingComponent : public USceneComponent, public IWeaponShootingInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UMeleeShootingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
private:
	UPROPERTY(EditAnywhere)
	TMap<EWeaponSoundType, FWeaponSoundBases> SoundMap;
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack", meta = (ClampMin = "10.0", UIMin = "10.0", ClampMax = "360.0", UIMax = "360.0"))
	float AttackAngle = 180.0f;

	
	/** 공격 판정의 정밀도를 결정합니다. 높을수록 정밀하지만 비용이 증가합니다. */
	UPROPERTY(EditDefaultsOnly, Category = "Melee Attack", meta = (ClampMin = "2", UIMin = "2"))
	int32 TraceSegments = 11;

	// --- Helper Functions ---
	void PerformFanTrace(const APlayerController* PlayerController, TArray<FHitResult>& OutRawHits) const;
	void FilterAndPrioritizeHits(const TArray<FHitResult>& RawHits, TMap<ABaseCharacter*, FHitResult>& OutBestHitPerCharacter) const;
	bool ApplyMeleeDamage(const TMap<ABaseCharacter*, FHitResult>& BestHits, AKillingFloorLikeCharacter* Character, const FWeaponData* WeaponData, bool IsSpecial) const;
	void PlayHitSound(bool bWasHit, const FVector& SoundLocation);
	
protected:
	UPROPERTY(EditAnywhere)
	float AttackRange = 200;
	
public:
	virtual bool Fire(AKillingFloorLikeCharacter* Character,FWeaponData* FWeaponData, bool IsSpecial) override;
	
	//normal -> index = 0
	//Hit 사운드는 0 = 안맞음
	// 1 = 무기체에 맞음
	// 2 = 몬스터에 맞음
	UFUNCTION(BlueprintCallable)
	USoundBase* GetSoundBase(EWeaponSoundType SoundType, int32 index);
		
};
