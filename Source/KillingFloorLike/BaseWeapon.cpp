// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseWeapon.h"

#include "InputTriggers.h"
#include "KFLikeGameInstance.h"
#include "TP_PickUpComponent.h"
#include "WeaponShootingInterface.h"
#include "Net/UnrealNetwork.h"
#include "NiagaraFunctionLibrary.h"
#include "PoolableComponent.h"
#include "ResourceManager.h"


// Sets default values
ABaseWeapon::ABaseWeapon()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bNetUseOwnerRelevancy = true;
	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	RootComponent = StaticMeshComponent;

	PoolableComponent = CreateDefaultSubobject<UPoolableComponent>(TEXT("PoolableComponent"));
}

// Called when the game starts or when spawned
void ABaseWeapon::BeginPlay()
{
	Super::BeginPlay();

	this->WeaponData = *Cast<UKFLikeGameInstance>(GetGameInstance())->GetWeaponData(Id);

	ShootingComponent = FindComponentByInterface<IWeaponShootingInterface>();
	PickUpComponent = GetComponentByClass<UTP_PickUpComponent>();
	//PickUpComponent->OnPickUp.AddDynamic(this, &ABaseWeapon::AttachWeaponToCharacter);

	//수류탄은 Frag으로
	UAnimMontage* AttackMontage = GetWeaponType() != EWeaponType::Grenade
		                              ? GetAnimation(EWeaponAnimationType::Fire, false, 0)
		                              : GetAnimation(EWeaponAnimationType::Frag, false, 0);
	SetAttackCooltime(AttackMontage);
	SetReplicateMovement(true);
	//this->WeaponData = *WeaponData;
	/*Name = WeaponData->name;
	Type = WeaponData->type;
	Perk = WeaponData->perk;
	CanSell = WeaponData->can_sell;
	AmmoCost = WeaponData->ammo_cost;
	AmmoCapacity = WeaponData->ammo_capacity;
	MagazineCapacity = WeaponData->magazine_capacity;
	Damage = WeaponData->damage;
	SpecialDamage = WeaponData->damage;
	HeadMultiplier = WeaponData->head_multiplier;
	Pellets = WeaponData->pellets;
	Spread = WeaponData->spread;
	RateOfFire = WeaponData->rate_of_fire;
	ReloadTime = WeaponData->reload_time;*/

	//Character가 할당된 채 Actor가 복제되면 onRep이 실행 안되는 문제때문에, Begin에서 실행.
	//2번 실행되는 경우가 존재하지만 내부에서 처리되어 문제 없음
	if (HasAuthority() == false)
	{
		OnRep_Character();
	}
}

void ABaseWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// CurrentTimeDilation 변수를 모든 클라이언트에게 복제합니다.
	DOREPLIFETIME(ABaseWeapon, WeaponData);
	DOREPLIFETIME(ABaseWeapon, CurrentAimType);
	DOREPLIFETIME(ABaseWeapon, Character);
}

// Called every frame
void ABaseWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (CurrentAttackCooltime > 0)
	{
		CurrentAttackCooltime -= DeltaTime;
	}
}

void ABaseWeapon::FireWeapon(bool IsSpecial)
{
	if (HasAuthority())
	{
		CheckFireWeapon(IsSpecial);
	}
}


bool ABaseWeapon::CheckFireWeapon(bool IsSpecial)
{
	bool result = false;
	if (IsValid(Character) == false || IsValid(Character->GetController()) == false)
	{
		return false;
	}

	if (ShootingComponent)
	{
		result = ShootingComponent->Fire(Character, &WeaponData, IsSpecial); // 현재 장착된 발사 방식을 실행
	}

	if (result)
	{
		Multi_FireWeaponCallback();
	}

	return result;
}

void ABaseWeapon::Multi_FireWeaponCallback_Implementation()
{
	if (IsValid(Character) == false)
	{
		return;
	}
	if (HasAuthority() || HasAuthority() == false && Character->IsLocallyControlled())
	{
		Character->DoRecoil();
	}
	USkeletalMeshComponent* SkelMesh = Character->GetMesh1P();
	if (IsValid(SkelMesh) == false || SkelMesh->DoesSocketExist(FName("tip")) == false)
	{
		return;
	}

	UStaticMeshComponent* TipComponent = GetStaticMeshComponent();
	if (IsValid(TipComponent) == false || TipComponent->DoesSocketExist(FName("tip")) == false)
	{
		return;
	}

	USceneComponent* AttachedComponent = nullptr;
	if (Character->IsLocallyControlled())
	{
		AttachedComponent = SkelMesh;
	}
	else
	{
		AttachedComponent = TipComponent;
	}

	//각 무기마다 다르게?
	UNiagaraSystem* MuzzleEffect = GetGameInstance()->GetSubsystem<UResourceManager>()->LoadEffect(
		TEXT("/Game/Effect/NS_Muzzle.NS_Muzzle"));
	// 1. 목표 회전 값 가져오기: 액터의 월드 회전
	const FRotator TargetRotation = GetCharacter()->GetActorRotation();

	// 2. 현재 기준이 되는 회전 값 가져오기: 'tip' 소켓의 월드 회전
	const FRotator SocketRotation = AttachedComponent->GetSocketRotation(FName("tip"));

	// 3. 목표 회전과 현재 회전의 차이(Delta)를 계산하여 필요한 상대 오프셋을 구합니다.
	const FRotator SpawnRotationOffset = TargetRotation + FRotator(0, -90, 0); //- SocketRotation;

	// 4. SpawnSystemAttached를 사용하되, 계산된 회전 오프셋을 적용합니다.
	UNiagaraComponent* NiagaraComp = UNiagaraFunctionLibrary::SpawnSystemAttached(
		MuzzleEffect,
		AttachedComponent,
		FName("tip"),
		AttachedComponent->GetSocketLocation(FName("tip")), // 위치는 소켓에 정확히 일치
		SpawnRotationOffset, // ✅ 계산된 회전 오프셋 적용
		EAttachLocation::KeepWorldPosition, // 상대 오프셋을 사용하므로 KeepRelativeOffset으로 변경
		true, // bAutoDestroy
		true, // bAutoActivate
		ENCPoolMethod::AutoRelease,
		true
	);
}

TSubclassOf<class UAnimInstance> ABaseWeapon::GetAnimInstance()
{
	return AnimInstance;
}

USkeletalMesh* ABaseWeapon::GetSkeletalMesh()
{
	return SkeletalMesh;
}

EWeaponType ABaseWeapon::GetWeaponType()
{
	return WeaponData.type;
}

UAnimMontage* ABaseWeapon::GetAnimation(EWeaponAnimationType AnimationType, bool IsIron, int index)
{
	if (AnimationMap.Contains(AnimationType) == false)
	{
		return nullptr;
	}

	if (IsIron && AnimationType == EWeaponAnimationType::Fire && AnimationMap[AnimationType].IronMontages.Num() > 0)
	{
		return AnimationMap[AnimationType].IronMontages[index];
	}
	else
	{
		return AnimationMap[AnimationType].Montages[index];
	}
}

UAnimMontage* ABaseWeapon::GetPlayerAnimation(EWeaponAnimationType AnimationType, bool IsSit)
{
	if (IsSit && PlayerAnimMontageMap.Contains(AnimationType))
	{
		int32 RandIdx = FMath::RandHelper(PlayerAnimMontageMap[AnimationType].IronMontages.Num());
		if (PlayerAnimMontageMap[AnimationType].IronMontages.IsValidIndex(RandIdx))
		{
			return PlayerAnimMontageMap[AnimationType].IronMontages[RandIdx];
		}
	}
	else
	{
		int32 RandIdx = FMath::RandHelper(PlayerAnimMontageMap[AnimationType].Montages.Num());
		if (PlayerAnimMontageMap[AnimationType].Montages.IsValidIndex(RandIdx))
		{
			return PlayerAnimMontageMap[AnimationType].Montages[RandIdx];
		}
	}

	return nullptr;
}

void ABaseWeapon::ClearPlayerAnimation()
{
	PlayerAnimMontageMap.Empty();
}


void ABaseWeapon::SetPlayerAnimation(UAnimMontage* PlayerMontage, EWeaponAnimationType AnimationType, bool IsSit)
{
	if (PlayerAnimMontageMap.Contains(AnimationType) == false)
	{
		PlayerAnimMontageMap.Add(AnimationType);
	}
	if (IsSit)
	{
		PlayerAnimMontageMap[AnimationType].IronMontages.Add(PlayerMontage);
	}
	else
	{
		PlayerAnimMontageMap[AnimationType].Montages.Add(PlayerMontage);
	}
}

//주사기?
bool ABaseWeapon::IsAttackable()
{
	return Character->IsControllable() && CurrentAttackCooltime <= 0;
}

void ABaseWeapon::SetCurrentAttackCooltime()
{
	float result = WeaponData.rate_of_fire;
	if (WeaponData.perk_type == Character->GetPerkData().type)
	{
		//TODO 조건이 다양함.
		result *= Character->GetPerkData().fire_rate;
	}
	CurrentAttackCooltime = result;
}

int32 ABaseWeapon::GetWeaponAnimationMaxIndex(EWeaponAnimationType type)
{
	if (AnimationMap.Contains(type) == false)
	{
		return 0;
	}
	if (CurrentAimType == EAimType::Iron && type == EWeaponAnimationType::Fire)
	{
		return AnimationMap[type].IronMontages.Num();
	}
	else
	{
		return AnimationMap[type].Montages.Num();
	}
}

EAimType ABaseWeapon::GetCurrentAimType()
{
	return CurrentAimType;
}

bool ABaseWeapon::IsAimTypeChangeable()
{
	if (Character->IsControllable() == false)
	{
		return false;
	}

	if (AnimationMap.Contains(EWeaponAnimationType::Fire) == false)
	{
		return false;
	}

	if (AnimationMap[EWeaponAnimationType::Fire].IronMontages.Num() > 0)
	{
		return true;
	}
	return false;
}

void ABaseWeapon::SetCurrentAimType(EAimType NewAimType)
{
	CurrentAimType = NewAimType;
}

int32 ABaseWeapon::GetAmmoMagCost()
{
	return WeaponData.ammo_cost * Character->GetDiscountRate(GetWeaponData().perk_type);
}

int32 ABaseWeapon::GetAmmoCapacity()
{
	return WeaponData.ammo_capacity;
}

int32 ABaseWeapon::GetMagazineCapacity()
{
	return WeaponData.magazine_capacity;
}

float ABaseWeapon::GetDamage()
{
	return WeaponData.damage;
}

float ABaseWeapon::GetSpecialDamage()
{
	return WeaponData.special_damage;
}

bool ABaseWeapon::GetIsAttackTrigger()
{
	return IsAttackTrigger;
}

int32 ABaseWeapon::GetId()
{
	return Id;
}

void ABaseWeapon::SetId(int32 NewId)
{
	Id = NewId;
}

FWeaponData ABaseWeapon::GetWeaponData()
{
	return WeaponData;
}

void ABaseWeapon::SetWeaponData(const FWeaponData& NewWeaponData)
{
	WeaponData = NewWeaponData;
}

/*USoundBase* ABaseWeapon::GetSoundBase(EWeaponSoundType SoundType, int32 index)
{
	return SoundMap[SoundType].SoundBases[index];
}*/

float ABaseWeapon::GetHeadMultiplier()
{
	return WeaponData.head_multiplier * Character->GetHeadMultiplier(this);
}

float ABaseWeapon::GetRecoil()
{
	return WeaponData.recoil * Character->GetRecoilMultiplier(this);
}

int32 ABaseWeapon::GetSellCost()
{
	return Character->GetWeaponCost(WeaponData) * 0.75f;
}

AKillingFloorLikeCharacter* ABaseWeapon::GetCharacter()
{
	return Character;
}

void ABaseWeapon::OnRep_Character()
{
	if (Character)
	{
		SetOwner(Character);
		/*AttachWeaponToCharacter(TODO);*/

		// Attach the weapon to the First Person Character
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::SnapToTarget, true);

		StaticMeshComponent->SetSimulatePhysics(false);
		StaticMeshComponent->SetVisibility(false);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		if (Character->GetMesh())
		{
			AttachToComponent(Character->GetMesh(), AttachmentRules, FName("WeaponR_Bone"));

			FTransform SocketTransform = StaticMeshComponent->GetSocketTransform(FName("WeaponSocket"), RTS_Actor);
			/*UE_LOG(LogTemp, Warning, TEXT("Before Relative SocketLocation : %s"),
			       *SocketTransform.GetLocation().ToString())*/
			//SetRelativeTransform(SocketTransform);
			SetActorRelativeTransform(SocketTransform);
			/*UE_LOG(LogTemp, Warning, TEXT("After GetActorLocation SocketLocation : %s"), *GetActorLocation().ToString())
			UE_LOG(LogTemp, Warning, TEXT("After Relative SocketLocation : %s"),
			       *SocketTransform.GetLocation().ToString())*/
		}
	}
	else
	{
		StaticMeshComponent->SetSimulatePhysics(true);
		StaticMeshComponent->SetVisibility(true);
		StaticMeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		SetOwner(nullptr);
	}
}

void ABaseWeapon::SetAttackCooltime(UAnimMontage* PlayedAnimMontage)
{
	if (IsValid(PlayedAnimMontage) == false)
	{
		return;
	}
	if (IsAsyncAttackCooltimeWithAnimation)
	{
		//MaxAttackCooltime = attack 애니메이션의 길이
		WeaponData.rate_of_fire = PlayedAnimMontage->GetPlayLength();
	}
	CurrentAttackCooltime = 0;
}

void ABaseWeapon::SetCharacter(AKillingFloorLikeCharacter* TargetCharacter)
{
	//서버에서는 바로 실행
	if (HasAuthority())
	{
		Character = TargetCharacter;

		OnRep_Character();
	}
}

void ABaseWeapon::AttachWeaponToCharacter(AKillingFloorLikeCharacter* TargetCharacter)
{
	//이미 소유자가 있는 무기
	if (IsValid(Character))
	{
		return;
	}

	//이미 있는 무기 유형
	if (IsValid(TargetCharacter->GetWeapon(this->GetWeaponType())))
	{
		return;
	}

	if(PickUpComponent){
	PickUpComponent->OnPickUp.RemoveDynamic(this, &ABaseWeapon::AttachWeaponToCharacter);
	}

	SetCharacter(TargetCharacter);


	TargetCharacter->Server_PickUpWeapon(this);
}

void ABaseWeapon::DropWeapon(AKillingFloorLikeCharacter* TargetCharacter)
{
	if (IsValid(Character) == false)
	{
		return;
	}


	SetCharacter(nullptr);

	SetActorLocation(TargetCharacter->GetActorLocation() + TargetCharacter->GetActorForwardVector() * 200, true,
	                 nullptr, ETeleportType::TeleportPhysics);
	StaticMeshComponent->AddImpulse(TargetCharacter->GetActorForwardVector() * 300);

	if (PickUpComponent)
	{
		PickUpComponent->OnPickUp.AddDynamic(this, &ABaseWeapon::AttachWeaponToCharacter);
	}
}

FText ABaseWeapon::GetAmmoText()
{
	return FText::FromString(""); // + " / " + FString::FromInt(SavedAmmo);
}

FText ABaseWeapon::GetClipText()
{
	return FText::FromString("");
}

void ABaseWeapon::Server_RequestFire_Implementation(ETriggerEvent TriggerEvent)
{
	// 1. 기존 Server_FireWeapon의 유효성 검사를 가져옵니다.
	if (!IsAttackable())
	{
		return;
	}

	if (!GetIsAttackTrigger() && TriggerEvent == ETriggerEvent::Triggered)
	{
		return;
	}

	// 2. 기존 Server_RequestShoot의 애니메이션 및 쿨다운 로직을 가져옵니다.
	// 캐릭터에 대한 참조가 필요합니다.
	if (AKillingFloorLikeCharacter* OwningCharacter = Cast<AKillingFloorLikeCharacter>(GetOwner()))
	{
		UAnimMontage* FireAnimMontage = GetWeaponMontage(EWeaponAnimationType::Fire);
		// FireAnimMontage가 null일 경우에 대한 처리는 GetWeaponMontage 내부 또는 여기서 수행할 수 있습니다.
		if (IsValid(FireAnimMontage) == false)
		{
			// 애니메이션 없이 바로 발사 로직을 실행하거나, 아무것도 하지 않을 수 있습니다.
			// 여기서는 기존 로직과 동일하게 return합니다.
			return;
		}

		UAnimMontage* PlayerFireAnimMontage = GetPlayerAnimation(EWeaponAnimationType::Fire, false);
		if (IsValid(PlayerFireAnimMontage) == false)
		{
			return;
		}
        
		// 캐릭터에게 애니메이션 재생을 요청합니다.
		OwningCharacter->Server_CheckPlayWeaponAnim(FireAnimMontage, PlayerFireAnimMontage, true);
	}

	// 자신의 쿨다운을 직접 설정합니다.
	SetCurrentAttackCooltime();

	// 참고: 실제 발사(ShootingComponent->Fire())는 애니메이션 노티파이에서 호출하는 것이 더 정확한 타이밍을 보장합니다.
	// 만약 즉시 발사해야 한다면 이 곳에서 호출할 수 있습니다.
}

UAnimMontage* ABaseWeapon::GetWeaponMontage(EWeaponAnimationType AnimationType)
{
	if (IsValid(Character) == false)
	{
		return nullptr;
	}
	int32 RandIdx = FMath::RandHelper(GetWeaponAnimationMaxIndex(AnimationType));
	return GetAnimation(AnimationType, Character->GetIsIron(), RandIdx);
}

