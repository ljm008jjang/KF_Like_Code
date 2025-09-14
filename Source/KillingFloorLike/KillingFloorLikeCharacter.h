// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "BaseWeapon.h"
#include "InputActionValue.h"
#include "KillingFloorLikeCharacter.generated.h"

struct FGameplayMessage_EndMatch;
struct FGameplayTag;
enum class ETriggerEvent : uint8;
struct FWeaponData;
enum class EPerkType : uint8;
enum class EWeaponType : uint8;
class UInputComponent;
class USkeletalMeshComponent;
class USceneComponent;
class UCameraComponent;
class UAnimMontage;
class USoundBase;
class ABaseWeapon;


/*UENUM(BlueprintType)
enum class EPerkType : uint8
{
	None,
	FieldMedic,
	SupportSpecialist,
	Sharpshooter,
	Commando,
	Berserker,
	Firebug,
	Demolitions
};*/

USTRUCT(BlueprintType)
struct FPerkData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Data")
	int32 id;
	// Perk 이름
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Data")
	EPerkType type;

	// Perk 랭크
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Data")
	int32 rank;

	// 재장전 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Data")
	float reload_rate;

	// 발사 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Data")
	float fire_rate;

	// 헤드샷 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Data")
	float headshot_damage;

	// Perk에 포함되지 않은 무기의 헤드샷 데미지
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Data")
	float off_perk_headshot_damage;

	// 반동
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Data")
	float recoil;

	// 일반 할인율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Data")
	float discount;

	// 특수 할인율
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Data")
	float special_discount;

	// 시작 무기 ID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Data")
	int32 spawn_weapon;

	FPerkData()
		: id(0)
		  , type(EPerkType::None)
		  , rank(0)
		  , reload_rate(0.0f)
		  , fire_rate(0.0f)
		  , headshot_damage(0.0f)
		  , off_perk_headshot_damage(0.0f)
		  , recoil(0.0f)
		  , discount(0.0f)
		  , special_discount(0.0f)
		  , spawn_weapon(0)
	{
	}
};

USTRUCT(BlueprintType)
struct FPerkConstData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Const Data")
	int32 id;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Const Data")
	EPerkType type;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Const Data")
	FString image_path;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Perk Const Data")
	FString gold_image_path;

	FPerkConstData()
		: id(0)
		  , type(EPerkType::None)
		  , image_path(TEXT(""))
		  , gold_image_path(TEXT(""))
	{
	}
};

USTRUCT(BlueprintType)
struct FPlayerPerk
{
	GENERATED_BODY()

	UPROPERTY()
	EPerkType Type;

	UPROPERTY()
	int32 Rank;

	FPlayerPerk()
		: Type(EPerkType::Sharpshooter)
		  , Rank(6)
	{
	}

	FPlayerPerk(EPerkType InType, int32 InRank)
		: Type(InType)
		  , Rank(InRank)
	{
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FChangeEventDispatcher);

UCLASS(config=Game)
class AKillingFloorLikeCharacter : public ABaseCharacter
{
	GENERATED_BODY()

	/** Pawn mesh: 1st person view (arms; seen only by self) */
	UPROPERTY(VisibleDefaultsOnly, Category=Mesh)
	USkeletalMeshComponent* Mesh1P;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* MoveAction;

	/** MappingContext */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	USkeletalMesh* Frag1PSkeletalMesh;

public:
	AKillingFloorLikeCharacter();
	virtual void PossessedBy(AController* NewController) override;

protected:
	virtual void BeginPlay();
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UFUNCTION(Server, Reliable)
	void Server_NotifyCharacterReady();

	virtual void OnRep_Controller() override;
	virtual void Tick(float DeltaSeconds) override;

public:
	/** Look Input Action */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	/** Bool for AnimBP to switch to another animation set */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon)
	bool bHasRifle;

	void OnGrenadeAnimationEnded(UAnimMontage* PlayedMontage, bool bInterrupted);
	/** Setter to set the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	void SetHasRifle(bool bNewHasRifle);

	/** Getter for the bool */
	UFUNCTION(BlueprintCallable, Category = Weapon)
	bool GetHasRifle();

	UFUNCTION(Server, Reliable, BlueprintCallable, Category = Weapon)
	void Server_PickUpWeapon(class ABaseWeapon* bNewHasRifle);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputMappingContext* FireMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* FireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SpecialFireAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ChangeAimTypeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* DropWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* GrenadeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* ReloadWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* SwapWeaponAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category=Input, meta=(AllowPrivateAccess = "true"))
	class UInputAction* FleshOrShopAction;

protected:
	/** Called for movement input */
	void Move(const FInputActionValue& Value);

	/** Called for looking input */
	void Look(const FInputActionValue& Value);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void Fire();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_CheckChangableAimType();
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_ChangeAimType();
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void EventChangeFov(bool IsIronState);
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_ReloadWeapon();
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_ReloadWeaponCallback();

	void DropWeapon();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DropWeapon(ABaseWeapon* DropWeapon);
	UFUNCTION(Server, Reliable)
	void Server_UseGrenade();

	ABaseWeapon* GetBestWeaponToEquip();
	UFUNCTION(BlueprintCallable)
	void GetNewWeapon(int32 WeaponId);

	UFUNCTION()
	void OnFireStarted();
	UFUNCTION()
	void OnFireTriggered();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_FireWeapon(ETriggerEvent TriggerEvent);
	UFUNCTION(Server, Reliable)
	void Server_SpecialFireWeapon();


	void GetSwapWeaponInputValue(const FInputActionValue& ActionValue);
	UFUNCTION(Server, Reliable)
	void Server_SwapWeaponCheck(int32 ActionValue);
	UFUNCTION(BlueprintCallable)
	EWeaponType IsSwapWeaponable(int32 ActionValue);

	UFUNCTION(Server, Reliable)
	void Server_SwapWeapon(ABaseWeapon* NewWeapon);
	UFUNCTION(NetMulticast, Reliable)
	void Multi_PlayPutDownAnimation(ABaseWeapon* NewWeapon, UAnimMontage* AnimMontage);

	void OnPutDownMontageEnded(bool bInterrupted, ABaseWeapon* NewWeapon);
	UFUNCTION(NetMulticast, Reliable)
	void Multi_OnPutDownMontageEndedCallback();


	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_PlayWeaponAnim(UAnimMontage* SelectMontage, UAnimMontage* PlayerMontage, float PlayRate = 1.0f);
	// APawn interface
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;
	// End of APawn interface

	UFUNCTION(Server, Reliable)
	void Server_OnSpecialOrShopStarted();
	UFUNCTION(Client, Reliable)
	void Client_OnSpecialOrShopStarted();

	virtual void OnRep_CurrentHealth() override;

public:
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	                         AActor* DamageCauser) override;
	virtual void Jump() override;

	/** Returns Mesh1P subobject **/
	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetMesh1P() const { return Mesh1P; }

	/** Returns FirstPersonCameraComponent subobject **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	//UFUNCTION(BlueprintCallable)
	/*ABaseWeapon* GetCurrnetWeapon()
	{
		return CurrentWeapon;	
	}*/
	UFUNCTION(BlueprintCallable)
	TArray<class ABaseWeapon*> GetWeaponArray();
	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FWeaponData> GetWeaponDataArray();
	UFUNCTION(BlueprintCallable)
	class ABaseWeapon* GetWeapon(EWeaponType WeaponType);
	UFUNCTION(BlueprintCallable)
	class ABaseWeapon* GetCurrentWeapon();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_SellWeapon(EWeaponType WeaponType);
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_PurchaseWeapon(int32 WeaponId);

	UFUNCTION(BlueprintSetter)
	void SetNextWeaponType(EWeaponType NewNextWeaponType);

	UFUNCTION(BlueprintGetter)
	int32 GetMoney() const;
	UFUNCTION(BlueprintGetter)
	int32 GetWeight() const;

	void SetMoney(int32 NewMoney)
	{
		if (HasAuthority())
		{
			Money = NewMoney;
			OnRep_Money();
		}
	}

	UPROPERTY(BlueprintAssignable, Category = "EventDispatcher")
	FChangeEventDispatcher PerkChangeEventDispatcher;
	UFUNCTION()
	void CallPerkChangeDispatcher()
	{
		PerkChangeEventDispatcher.Broadcast();
	}

	UFUNCTION(BlueprintCallable)
	FString GetHPText();
	UFUNCTION(BlueprintCallable)
	FString GetArmorText();
	UFUNCTION(BlueprintCallable)
	FText GetWeightText();

	UFUNCTION(BlueprintGetter)
	FPerkData GetPerkData();

	UFUNCTION(BlueprintGetter)
	EPerkType GetPerkType()
	{
		return PlayerPerkData.type;
	}

	UFUNCTION(BlueprintGetter)
	int32 GetPerkRank()
	{
		return PlayerPerkData.rank;
	}

	UFUNCTION(BlueprintGetter)
	bool GetIsChangablePerk();
	UFUNCTION(BlueprintSetter)
	void SetIsChangablePerk(bool NewIsChangablePerk);
	UFUNCTION(BlueprintGetter)
	bool GetIsShopable()
	{
		return IsShopable;
	}

	UFUNCTION(BlueprintSetter)
	void SetIsShopable(bool NewIsChangablePerk)
	{
		IsShopable = NewIsChangablePerk;
	}

	float GetHeadMultiplier(ABaseWeapon* Weapon);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetReloadRate(ABaseWeapon* Weapon);
	//UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetRecoilMultiplier(ABaseWeapon* Weapon);
	// float GetDiscountRate(ABaseWeapon* Weapon);
	float GetDiscountRate(EPerkType WeaponPerkType);
	UFUNCTION(BlueprintPure)
	int32 GetWeaponCost(const FWeaponData& WeaponData);
	UFUNCTION(BlueprintCallable)
	bool IsBuyable(const FWeaponData& WeaponData);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void DoRecoil();

	UFUNCTION(BlueprintCallable)
	void CalculateWeight();

	UFUNCTION(BlueprintGetter)
	float GetMaxArmor();

	UFUNCTION(BlueprintGetter)
	float GetCurrentArmor();

	UFUNCTION(Server, Reliable)
	void Server_ChangePerk(EPerkType Type, int32 Rank);

	UFUNCTION()
	bool GetIsIron()
	{
		return IsIron;
	}

	UFUNCTION()
	void SetIsIron(bool bIsIron)
	{
		if (HasAuthority())
		{
			IsIron = bIsIron;
			OnRep_IsIron();
		}
	}

	UFUNCTION(BlueprintGetter)
	class APlayerCharacterController* GetPlayerCharacterController() const
	{
		return PlayerCharacterController;
	}

	UFUNCTION(Server, Reliable)
	void Server_BuyWeaponMag(ABaseWeapon* Weapon);
	/*UFUNCTION(Server, Reliable)
	void Server_BuyWeaponFill(ABaseWeapon* Weapon);*/
	UFUNCTION(NetMulticast,Reliable)
	void Multi_RestartCallback();
	UFUNCTION(Server,Reliable)
	void Server_BuyArmor();
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_CheckPlayWeaponAnim(UAnimMontage* WeaponMontage, UAnimMontage* PlayerMontage,
									bool IsStopPlayingMontage = false, float PlayRate = 1.0f);
private:
	UPROPERTY()
	class UKFLikeGameInstance* GameInstance;
	UKFLikeGameInstance* GetKFLikeGameInstance();

	UPROPERTY(BlueprintGetter = GetPlayerCharacterController)
	class APlayerCharacterController* PlayerCharacterController;
	UPROPERTY()
	class UPlayerAnimInstance* PlayerAnimInstance;

	UPROPERTY(Replicated, VisibleAnywhere)
	TArray<class ABaseWeapon*> WeaponInventory;
	/*UFUNCTION()
	void OnRep_WeaponInventory();*/

	void AddWeaponInventory(ABaseWeapon* NewWeapon)
	{
		WeaponInventory.Add(NewWeapon);
		if (HasAuthority())
		{
			CalculateWeight();
			//OnRep_WeaponInventory();
		}
	}

	void RemoveWeaponInventory(ABaseWeapon* NewWeapon)
	{
		WeaponInventory.Remove(NewWeapon);
		if (HasAuthority())
		{
			//OnRep_WeaponInventory();
			CalculateWeight();
		}
	}

	/*void RemoveWeaponInventory(ABaseWeapon* NewWeapon)
	{
		WeaponInventory.Add(NewWeapon);
		if (HasAuthority())
		{
			OnRep_WeaponInventory();
		}
	}*/
	void CheckAllWeaponsReadyAndProcess();
	FTimerHandle TimerHandle_CheckWeaponsReady;

	UPROPERTY(EditAnywhere)
	TArray<int32> BaseWeaponIds;
	UPROPERTY(ReplicatedUsing=OnRep_CurrentWeapon)
	ABaseWeapon* CurrentWeapon;
	UFUNCTION()
	void OnRep_CurrentWeapon();

	void SetCurrentWeapon(ABaseWeapon* NewWeapon)
	{
		if (HasAuthority())
		{
			CurrentWeapon = NewWeapon;
			OnRep_CurrentWeapon();
		}
	}


	UPROPERTY(ReplicatedUsing=OnRep_Money, EditAnywhere, BlueprintGetter = GetMoney)
	int32 Money = 100;


	UFUNCTION()
	void OnRep_Money();
	UPROPERTY(EditDefaultsOnly, Category="Stat")
	int32 MaxWeight = 15;
	UPROPERTY(ReplicatedUsing=OnRep_Weight, EditAnywhere, BlueprintGetter = GetWeight)
	int32 Weight = 0;

	void SetWeight(int32 NewWeight)
	{
		if (HasAuthority())
		{
			Weight = NewWeight;
			OnRep_Weight();
		}
	}

	UFUNCTION()
	void OnRep_Weight();

	UPROPERTY(EditDefaultsOnly, BlueprintGetter = GetMaxArmor, Category="Stat")
	float MaxArmor = 100;
	UPROPERTY(ReplicatedUsing=OnRep_CurrentHealth, EditAnywhere, BlueprintGetter = GetCurrentArmor, Category="Stat")
	float CurrentArmor;
	int32 ArmorCost = 300;
	void SetCurrentArmor(float NewCurrentArmor)
	{
		if (HasAuthority())
		{
			CurrentArmor = NewCurrentArmor;
			OnRep_CurrentHealth();
		}
	}


	UPROPERTY(ReplicatedUsing = OnRep_PlayerPerkData, BlueprintGetter = GetPerkData, Category="Data")
	FPerkData PlayerPerkData;

	void SetPlayerPerkData(const FPerkData& NewPlayerPerk)
	{
		if (HasAuthority())
		{
			PlayerPerkData = NewPlayerPerk;
			OnRep_PlayerPerkData();
		}
	}

	UFUNCTION()
	void OnRep_PlayerPerkData();
	UPROPERTY(Replicated, EditAnywhere, BlueprintGetter = GetIsChangablePerk, BlueprintSetter = SetIsChangablePerk)
	bool IsChangablePerk = true;
	UPROPERTY(Replicated)
	bool IsShopable = false;

	float ElapsedTime = 0;
	UPROPERTY()
	FVector FirstMeshLocation;

	UFUNCTION(BlueprintCallable)
	void SwapWeaponCallback(ABaseWeapon* Weapon);
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_SwapWeaponCallback(ABaseWeapon* Weapon);
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_DelayedSwapWeaponCallback(ABaseWeapon* Weapon);

	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_DropMoney();

	UFUNCTION(Client, Reliable)
	void Client_PlayHitEffect(FDamageEvent const& DamageEvent, float NewArmor, float NewHp);

	UPROPERTY(ReplicatedUsing=OnRep_IsIron)
	bool IsIron = false;
	UFUNCTION()
	void OnRep_IsIron();

	UFUNCTION(Server, Reliable)
	void Server_RequestShoot();

	void EndGameCallback(FGameplayTag Channel, const FGameplayMessage_EndMatch& Message);

};
