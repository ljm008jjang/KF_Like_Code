// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BaseWeapon.generated.h"

enum class ETriggerEvent : uint8; 
class UPoolableComponent;
class UTP_PickUpComponent;
class UNiagaraSystem;
//enum class EPerkType : uint8;
class IWeaponShootingInterface;

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	None, // 0 - 우선순위 가장 낮음
	Main, // 3 - 주 무기
	Sub, // 2 - 보조 무기
	Knife, // 1 - 근접 무기
	Syringe, // 4 - 특수 장비 (가장 높은 우선순위 또는 별도 처리)
	Grenade, // 5 - 폭탄
};

UENUM(BlueprintType)
enum class EWeaponAnimationType : uint8
{
	None,
	Fire,
	Select,
	PutDown,
	Reload,
	SpecialFire,
	Frag
};

UENUM(BlueprintType)
enum class EAimType : uint8
{
	None,
	Normal,
	Iron
};


USTRUCT(BlueprintType)
struct FAttackMontages
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<UAnimMontage*> Montages;
	UPROPERTY(EditAnywhere)
	TArray<UAnimMontage*> IronMontages;
};

UENUM(BlueprintType)
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
};

ENUM_RANGE_BY_COUNT(EPerkType, EPerkType::Demolitions)


USTRUCT(BlueprintType)
struct FWeaponData : public FTableRowBase
{
	GENERATED_BODY()

public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	EWeaponType type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	EPerkType perk_type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	bool can_sell;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 price;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 weight;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 ammo_cost;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 ammo_capacity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 magazine_capacity;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 ammo_consume;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float special_damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 ammo_consume_special;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float head_multiplier;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 pellets;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float spread;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float rate_of_fire;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float reload_time;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float recoil;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 penetration;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	float penetration_damage;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString image_path;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString animation_name;

	bool operator==(const FWeaponData& Other) const
	{
		return id == Other.id;
	}

	FWeaponData()
		: id(0)
		  , name(TEXT(""))
		  , type(EWeaponType::None)
		  , perk_type(EPerkType::None)
		  , can_sell(false)
		  , price(0)
		  , weight(0)
		  , ammo_cost(0)
		  , ammo_capacity(0)
		  , magazine_capacity(0)
		  , damage(0.0f)
		  , ammo_consume(0.0f)
		  , special_damage(0.0f)
		  , ammo_consume_special(0.0f)
		  , head_multiplier(1.0f)
		  , pellets(1)
		  , spread(0.0f)
		  , rate_of_fire(1.0f)
		  , reload_time(1.0f)
		  , recoil(0.0f)
		  , penetration(0)
		  , penetration_damage(0.0f)
		  , image_path(TEXT(""))
		  , animation_name(TEXT(""))
	{
	}
};

UCLASS()
class KILLINGFLOORLIKE_API ABaseWeapon : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseWeapon();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void SetCharacter(class AKillingFloorLikeCharacter* TargetCharacter);

	UFUNCTION()
	void AttachWeaponToCharacter(AKillingFloorLikeCharacter* TargetCharacter);

	UFUNCTION(BlueprintCallable, Category="Weapon")
	void DropWeapon(class AKillingFloorLikeCharacter* TargetCharacter);

	/** Make the weapon Fire a Projectile */
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void FireWeapon(bool IsSpecial);

	UFUNCTION(BlueprintCallable, Category="Weapon")
	virtual bool CheckFireWeapon(bool IsSpecial);

	UFUNCTION(NetMulticast, Reliable, Category="Weapon")
	void Multi_FireWeaponCallback();

	TSubclassOf<class UAnimInstance> GetAnimInstance();

	UFUNCTION(BlueprintGetter)
	USkeletalMesh* GetSkeletalMesh();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	EWeaponType GetWeaponType();

	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetAnimation(EWeaponAnimationType AnimationType, bool IsIron, int index);

	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetPlayerAnimation(EWeaponAnimationType AnimationType, bool IsSit = false);
	void ClearPlayerAnimation();
	void SetPlayerAnimation(UAnimMontage* PlayerMontage, EWeaponAnimationType AnimationType, bool IsSit);

	UFUNCTION(BlueprintCallable)
	virtual bool IsAttackable();

	UFUNCTION(BlueprintCallable)
	void SetCurrentAttackCooltime();

	UFUNCTION(BlueprintCallable)
	int32 GetWeaponAnimationMaxIndex(EWeaponAnimationType type);

	UFUNCTION(BlueprintCallable)
	EAimType GetCurrentAimType();

	UFUNCTION(BlueprintCallable)
	bool IsAimTypeChangeable();

	UFUNCTION(BlueprintCallable)
	void SetCurrentAimType(EAimType NewAimType);

	UFUNCTION(BlueprintCallable, Blueprintpure)
	int32 GetAmmoMagCost();

	UFUNCTION(BlueprintCallable, Blueprintpure)
	int32 GetAmmoCapacity();

	UFUNCTION(BlueprintCallable, Blueprintpure)
	int32 GetMagazineCapacity();

	UFUNCTION(BlueprintCallable, Blueprintpure)
	float GetDamage();

	UFUNCTION(BlueprintCallable, Blueprintpure)
	float GetSpecialDamage();

	UFUNCTION(BlueprintCallable, Blueprintpure)
	bool GetIsAttackTrigger();

	int32 GetId();
	void SetId(int32 NewId);

	UFUNCTION(BlueprintCallable, Blueprintpure)
	FWeaponData GetWeaponData();

	void SetWeaponData(const FWeaponData& NewWeaponData);

	UFUNCTION(BlueprintGetter)
	float GetHeadMultiplier();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetRecoil();

	//UFUNCTION(BlueprintCallable, BlueprintPure)
	int32 GetSellCost();

	/** The Character holding this weapon*/
	UFUNCTION(BlueprintCallable, BlueprintPure)
	class AKillingFloorLikeCharacter* GetCharacter();

	UStaticMeshComponent* GetStaticMeshComponent()
	{
		return StaticMeshComponent;
	}

	UFUNCTION(BlueprintCallable)
	virtual FText GetAmmoText();
	UFUNCTION(BlueprintCallable)
	virtual FText GetClipText();
	UAnimMontage* GetWeaponMontage(EWeaponAnimationType AnimationType);

	UFUNCTION(Server, Reliable)
	void Server_RequestFire(ETriggerEvent TriggerEvent);
protected:
	/** The Character holding this weapon*/
	UPROPERTY(ReplicatedUsing=OnRep_Character)
	class AKillingFloorLikeCharacter* Character;
	UFUNCTION()
	void OnRep_Character();

	UPROPERTY(EditAnywhere)
	TMap<EWeaponAnimationType, FAttackMontages> AnimationMap;


	UPROPERTY(EditAnywhere)
	TMap<EWeaponAnimationType, FAttackMontages> PlayerAnimMontageMap;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPoolableComponent* PoolableComponent;

private:
	UPROPERTY(EditAnywhere)
	TSubclassOf<class UAnimInstance> AnimInstance;

	UPROPERTY(EditAnywhere, BlueprintGetter=GetSkeletalMesh)
	USkeletalMesh* SkeletalMesh;

	IWeaponShootingInterface* ShootingComponent; // 인터페이스로 설정
	UPROPERTY()
	UTP_PickUpComponent* PickUpComponent;


	UPROPERTY(Replicated, EditDefaultsOnly)
	EAimType CurrentAimType = EAimType::Normal;

	//true 시 공격 쿨타임이 애니메이션과 동기화 ex)Knife
	UPROPERTY(EditDefaultsOnly)
	bool IsAsyncAttackCooltimeWithAnimation = true;
	//클릭 시 자동으로 fire
	UPROPERTY(EditDefaultsOnly, BlueprintGetter=GetIsAttackTrigger)
	bool IsAttackTrigger = true;

	UPROPERTY(EditAnywhere, Category="Data")
	int32 Id = 1;
	UPROPERTY(Replicated, EditAnywhere, Category="Data")
	FWeaponData WeaponData;

	UPROPERTY(VisibleAnywhere, Category="Stat")
	float CurrentAttackCooltime;

	void SetAttackCooltime(UAnimMontage* PlayedAnimMontage);
};
