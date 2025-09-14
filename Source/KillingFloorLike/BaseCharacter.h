// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "BaseCharacter.generated.h"

UENUM(BlueprintType)
enum class EUnitState : uint8
{
	None,
	Idle,
	Walk,
	Run,
	Dead,
	UnMovable
};

UENUM(BlueprintType)
enum class EKFUnitType : uint8
{
	None,
	Ally,
	Enemy,
	Neutral
};

UENUM(BlueprintType)
enum class EMonsterSoundType : uint8
{
	None,
	Idle,
	Walk,
	Run,
	Attack,
	Hit,
	Dead,
	//상대방 피격 시 소리
	AttackEnemy
};

UCLASS()
class KILLINGFLOORLIKE_API ABaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY(ReplicatedUsing=OnRep_UnitState, VisibleAnywhere, BlueprintGetter = GetCurrentUnitState)
	EUnitState CurrentUnitState = EUnitState::None;
	UFUNCTION()
	void OnRep_UnitState();

	UPROPERTY(EditAnywhere)
	TMap<EMonsterSoundType, USoundBase*> SoundMap;
	const float HeadshotScale = 1.1f;

protected:
	UPROPERTY(EditAnywhere, Category="Stat")
	float BaseAttackDamage = 10;
	UPROPERTY(Replicated, EditDefaultsOnly, BlueprintGetter = GetMaxHP, Category="Stat")
	float MaxHp = 100;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHealth, EditAnywhere, BlueprintGetter = GetCurrentHP, Category="Stat")
	float CurrentHp;

	void SetCurrentHp(float NewHp)
	{
		if (HasAuthority())
		{
			CurrentHp = NewHp;
			OnRep_CurrentHealth();
		}
	}

	UPROPERTY(EditAnywhere, Category="Stat")
	float WalkSpeed;
	UPROPERTY(EditAnywhere, Category="Stat")
	float RunSpeed;

	UPROPERTY(Replicated, VisibleAnywhere)
	EKFUnitType CurrentUnitType = EKFUnitType::None;

	UPROPERTY(ReplicatedUsing=OnRep_AudioComponent)
	UAudioComponent* AudioComponent;
	UFUNCTION()
	void OnRep_AudioComponent();


	virtual void Dead(AActor* DamageCauser);
	UFUNCTION(NetMulticast, Reliable)
	virtual void Multi_DeadCallback(AActor* DamageCauser);


	float CalcFinalDamageAmount(FDamageEvent const& DamageEvent, float amount);

	UFUNCTION(BlueprintCallable)
	virtual void Attack();

	UFUNCTION()
	virtual void OnRep_CurrentHealth();

public:
	UFUNCTION(BlueprintCallable)
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	                         AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable)
	bool IsControllable();
	bool IsDead();
	UFUNCTION(BlueprintGetter)
	float GetMaxHP();

	UFUNCTION(BlueprintGetter)
	float GetCurrentHP();

	UFUNCTION(BlueprintCallable)
	float GetHpRatio();

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ChangeUnitState(EUnitState NewUnitState, bool IsHard = false);

	UFUNCTION(BlueprintGetter)
	EUnitState GetCurrentUnitState();

	UFUNCTION(BlueprintCallable)
	bool GetIsAlive();

	EKFUnitType GetCurrentUnitType();

	UFUNCTION(BlueprintCallable)
	bool IsAttackableUnitType(ABaseCharacter* AttackedUnit);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_PlaySoundBase(EMonsterSoundType SoundType);

	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_PlayCharacterAnim(UAnimMontage* SelectMontage, bool IsStopOtherMontage = false);

	/*UFUNCTION(NetMulticast, Reliable)
	void Multi_SetAudioComponent(UAudioComponent* NewAudioComponent);*/
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayAndSetSound(USoundBase* Sound);
	UFUNCTION(NetMulticast, Reliable)
	void Multi_StopAudioComponent();
};
