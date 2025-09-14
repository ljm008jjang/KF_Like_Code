// Copyright Epic Games, Inc. All Rights Reserved.

#include "KillingFloorLikeCharacter.h"

#include "CrossHair.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "HealDamageType.h"
#include "KFLikeGameInstance.h"
#include "KillingFloorHud.h"
#include "MoneyObject.h"
#include "ObjectPoolManager.h"
#include "PlayerAnimInstance.h"
#include "PlayerCharacterController.h"
#include "PubSubManager.h"
#include "RangeWeapon.h"
#include "ResourceManager.h"
#include "ShopWidget.h"
#include "TP_PickUpComponent.h"
#include "WeaponFleshComponent.h"
#include "WeaponSkillInterface.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"


//////////////////////////////////////////////////////////////////////////
// AKillingFloorLikeCharacter

class UGameplayMessageSubsystem;
class UTP_PickUpComponent;

AKillingFloorLikeCharacter::AKillingFloorLikeCharacter()
{
	// Character doesnt have a rifle at start
	bHasRifle = false;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);

	// Create a CameraComponent	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCameraComponent->SetupAttachment(GetCapsuleComponent());
	FirstPersonCameraComponent->SetRelativeLocation(FVector(-10.f, 0.f, 60.f)); // Position the camera
	FirstPersonCameraComponent->bUsePawnControlRotation = true;

	// Create a mesh component that will be used when being viewed from a '1st person' view (when controlling this pawn)
	Mesh1P = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("CharacterMesh1P"));
	Mesh1P->SetOnlyOwnerSee(true);
	Mesh1P->SetupAttachment(FirstPersonCameraComponent);
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->CastShadow = false;
	//Mesh1P->SetRelativeRotation(FRotator(0.9f, -19.19f, 5.2f));
	Mesh1P->SetRelativeLocation(FVector(-30.f, 0.f, -150.f));
	//Mesh1P->SetIsReplicated(true);

	CurrentUnitType = EKFUnitType::Ally;
	bReplicates = true;
	bAlwaysRelevant = true;
}

void AKillingFloorLikeCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	PlayerCharacterController = Cast<APlayerCharacterController>(GetController());
}

void AKillingFloorLikeCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// CurrentTimeDilation 변수를 모든 클라이언트에게 복제합니다.
	DOREPLIFETIME(AKillingFloorLikeCharacter, IsShopable);
	DOREPLIFETIME(AKillingFloorLikeCharacter, CurrentArmor);
	DOREPLIFETIME(AKillingFloorLikeCharacter, Money);
	DOREPLIFETIME(AKillingFloorLikeCharacter, WeaponInventory);
	DOREPLIFETIME(AKillingFloorLikeCharacter, PlayerPerkData);
	DOREPLIFETIME(AKillingFloorLikeCharacter, Weight);
	DOREPLIFETIME(AKillingFloorLikeCharacter, CurrentWeapon);
	DOREPLIFETIME(AKillingFloorLikeCharacter, IsIron);
}

void AKillingFloorLikeCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();
	SetReplicateMovement(true);
	//GameInstance = Cast<UKFLikeGameInstance>(GetGameInstance());
	PlayerAnimInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
	FirstMeshLocation = GetMesh1P()->GetRelativeLocation();


	SetCurrentArmor(100);


	// 서버에서만 퍽을 권위있게 설정합니다.
	if (HasAuthority())
	{
		// IsLocallyControlled는 이 캐릭터가 로컬 플레이어에 의해 제어되는지 확인합니다.
		// 리슨 서버 환경에서, 이 함수는 호스트의 캐릭터에 대해서만 true를 반환합니다.
		if (IsLocallyControlled())
		{
			// 호스트 플레이어에게는 SupportSpecialist 퍽을 할당합니다.
			SetPlayerPerkData(*GetKFLikeGameInstance()->GetPerkData(EPerkType::SupportSpecialist, 6));
		}
		else
		{
			// 다른 모든 플레이어(클라이언트)에게는 Sharpshooter 퍽을 할당합니다.
			SetPlayerPerkData(*GetKFLikeGameInstance()->GetPerkData(EPerkType::Sharpshooter, 6));
		}
	}

	GetGameInstance()->GetSubsystem<UPubSubManager>()->Subscribe(EPubSubTag::Mode_End, this,
	                                                             &AKillingFloorLikeCharacter::EndGameCallback);


	//Add Input Mapping Context
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<
			UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			Subsystem->AddMappingContext(FireMappingContext, 1);
		}
	}

	if (IsLocallyControlled())
	{
		Server_NotifyCharacterReady();
		//이상하게 monster도 서버에서 IsLocallyControlled라고 함.

		GetMesh()->SetVisibility(false);
		GetMesh()->SetHiddenInGame(true);
	}
}

void AKillingFloorLikeCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (HasAuthority())
	{
		// 인벤토리를 순회하며 모든 무기를 명시적으로 파괴합니다.
		for (ABaseWeapon* Weapon : WeaponInventory)
		{
			if (Weapon)
			{
				// 서버에서 Destroy()를 호출하면, 이 명령이 모든 클라이언트에게 복제됩니다.
				Weapon->Destroy();
			}
		}
		WeaponInventory.Empty();
	}

	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CheckWeaponsReady);
	Super::EndPlay(EndPlayReason);
}

void AKillingFloorLikeCharacter::Server_NotifyCharacterReady_Implementation()
{
	if (BaseWeaponIds.IsEmpty() == false)
	{
		for (auto BaseWeaponId : BaseWeaponIds)
		{
			GetNewWeapon(BaseWeaponId);
		}
	}
	if (PlayerPerkData.spawn_weapon > 0)
	{
		GetNewWeapon(PlayerPerkData.spawn_weapon);
	}
}

void AKillingFloorLikeCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	PlayerCharacterController = Cast<APlayerCharacterController>(GetController());
}

void AKillingFloorLikeCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (GetVelocity().SquaredLength() >= pow(GetCharacterMovement()->MaxWalkSpeedCrouched, 2))
	{
		ElapsedTime += DeltaSeconds;
		float degree = fmod(ElapsedTime * 360, 360);

		float radian = FMath::DegreesToRadians(degree);

		float cosD = cos(radian);
		float sinD = sin(radian);
		float deltaY = cosD * DeltaSeconds * 5;
		float deltaZ = sinD * cosD * DeltaSeconds * 5;
		FVector deltaVec(0, deltaY, deltaZ);
		GetMesh1P()->AddRelativeLocation(deltaVec);
	}
	else
	{
		ElapsedTime = 0;


		//ElapsedTime = FMath::Clamp(ElapsedTime, 0, 1);

		FVector NewLocation = FMath::Lerp(GetMesh1P()->GetRelativeLocation(), FirstMeshLocation,
		                                  1.0f);
		GetMesh1P()->SetRelativeLocation(NewLocation);
	}
}

//////////////////////////////////////////////////////////////////////////// Input


void AKillingFloorLikeCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this,
		                                   &AKillingFloorLikeCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this,
		                                   &AKillingFloorLikeCharacter::Look);

		// DropWeapon
		EnhancedInputComponent->BindAction(DropWeaponAction, ETriggerEvent::Started, this,
		                                   &AKillingFloorLikeCharacter::DropWeapon);
		EnhancedInputComponent->BindAction(GrenadeAction, ETriggerEvent::Started, this,
		                                   &AKillingFloorLikeCharacter::Server_UseGrenade);

		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this,
		                                   &AKillingFloorLikeCharacter::OnFireStarted);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this,
		                                   &AKillingFloorLikeCharacter::OnFireTriggered);

		EnhancedInputComponent->BindAction(SwapWeaponAction, ETriggerEvent::Started, this,
		                                   &AKillingFloorLikeCharacter::GetSwapWeaponInputValue);

		EnhancedInputComponent->BindAction(ChangeAimTypeAction, ETriggerEvent::Started, this,
		                                   &AKillingFloorLikeCharacter::Server_CheckChangableAimType);

		// Reload
		EnhancedInputComponent->BindAction(ReloadWeaponAction, ETriggerEvent::Started, this,
		                                   &AKillingFloorLikeCharacter::Server_ReloadWeapon);

		EnhancedInputComponent->BindAction(SpecialFireAction, ETriggerEvent::Started, this,
		                                   &AKillingFloorLikeCharacter::
		                                   Server_SpecialFireWeapon);

		EnhancedInputComponent->BindAction(FleshOrShopAction, ETriggerEvent::Started, this,
		                                   &AKillingFloorLikeCharacter::Server_OnSpecialOrShopStarted);
	}
}

void AKillingFloorLikeCharacter::Server_OnSpecialOrShopStarted_Implementation()
{
	if (IsControllable() == false)
	{
		return;
	}
	if (IsShopable)
	{
		Client_OnSpecialOrShopStarted();
		Server_ChangeUnitState(EUnitState::UnMovable);
	}
	else
	{
		ABaseWeapon* CurWeapon = GetCurrentWeapon();
		if (UWeaponFleshComponent* FleshComponent = CurWeapon->GetComponentByClass<UWeaponFleshComponent>())
		{
			Multi_PlayWeaponAnim(FleshComponent->GetFleshAnimMontage(), nullptr);
		}
	}
}

void AKillingFloorLikeCharacter::Client_OnSpecialOrShopStarted_Implementation()
{
	if (IsLocallyControlled() == false)
	{
		return;
	}

	UShopWidget* ShopInstance = Cast<UShopWidget>(
		PlayerCharacterController->GetKillingFloorHud()->GetWbShopInstance());
	if (ShopInstance == nullptr)
	{
		return;
	}
	if (ShopInstance->IsVisible())
	{
		ShopInstance->RemoveFromParent();
	}
	else
	{
		ShopInstance->AddToViewport();
		ShopInstance->OnEnter();
		//ChangeUnitState(EUnitState::UnMovable);
	}
}

void AKillingFloorLikeCharacter::OnRep_CurrentHealth()
{
	Super::OnRep_CurrentHealth();
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Player_HealthChange, this,
	                                                           FGameplayMessage_PlayerHeathChange(
		                                                           this, GetCurrentHP(), GetCurrentArmor()));
}


float AKillingFloorLikeCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
                                             AController* EventInstigator, AActor* DamageCauser)
{
	float realDamageAmount = AActor::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	float damage = CalcFinalDamageAmount(DamageEvent, realDamageAmount);

	//힐이면 체력 회복.
	if (DamageEvent.DamageTypeClass == UHealDamageType::StaticClass())
	{
		SetCurrentHp(GetCurrentHP() + damage);
	}
	else
	{
		//damage가 음수이므로
		if (CurrentArmor > -damage)
		{
			UE_LOG(LogTemp, Display, TEXT("1%f"), damage);
			SetCurrentArmor(GetCurrentArmor() + damage);
		}
		else
		{
			UE_LOG(LogTemp, Display, TEXT("2%f"), damage);
			damage += CurrentArmor;
			SetCurrentArmor(0);
			SetCurrentHp(GetCurrentHP() + damage);
		}
	}
	UE_LOG(LogTemp, Display, TEXT("Server_PlayHitEffect : %f, %f"), CurrentArmor, CurrentHp);
	Client_PlayHitEffect(DamageEvent, CurrentArmor, CurrentHp);

	if (CurrentHp <= 0)
	{
		Dead(DamageCauser);
	}
	else if (CurrentHp >= MaxHp)
	{
		SetCurrentHp(MaxHp);
	}

	return realDamageAmount;
}

void AKillingFloorLikeCharacter::Jump()
{
	if (IsControllable() == false)
	{
		return;
	}

	Super::Jump();
}

TArray<ABaseWeapon*> AKillingFloorLikeCharacter::GetWeaponArray()
{
	return WeaponInventory;
}

TArray<FWeaponData> AKillingFloorLikeCharacter::GetWeaponDataArray()
{
	TArray<FWeaponData> WeaponDataArray;
	for (auto Tuple : WeaponInventory)
	{
		WeaponDataArray.Add(Tuple->GetWeaponData());
	}
	return WeaponDataArray;
}

ABaseWeapon* AKillingFloorLikeCharacter::GetWeapon(EWeaponType WeaponType)
{
	for (auto Weapon : WeaponInventory)
	{
		if (Weapon != nullptr && Weapon->GetWeaponType() == WeaponType)
		{
			return Weapon;
		}
	}
	return nullptr;
}

ABaseWeapon* AKillingFloorLikeCharacter::GetCurrentWeapon()
{
	return CurrentWeapon;
}


void AKillingFloorLikeCharacter::SetNextWeaponType(EWeaponType NewNextWeaponType)
{
	//NextWeaponType = NewNextWeaponType;
}

int32 AKillingFloorLikeCharacter::GetMoney() const
{
	return Money;
}

int32 AKillingFloorLikeCharacter::GetWeight() const
{
	return Weight;
}

FString AKillingFloorLikeCharacter::GetHPText()
{
	return FString::FromInt(GetCurrentHP());
}

FString AKillingFloorLikeCharacter::GetArmorText()
{
	return FString::FromInt(GetCurrentArmor());
}

FText AKillingFloorLikeCharacter::GetWeightText()
{
	return FText::FromString(FString::FromInt(GetWeight()) + "/" + FString::FromInt(MaxWeight));
}

FPerkData AKillingFloorLikeCharacter::GetPerkData()
{
	return PlayerPerkData;
}

bool AKillingFloorLikeCharacter::GetIsChangablePerk()
{
	return IsChangablePerk;
}

void AKillingFloorLikeCharacter::SetIsChangablePerk(bool NewIsChangablePerk)
{
	IsChangablePerk = NewIsChangablePerk;
}

float AKillingFloorLikeCharacter::GetHeadMultiplier(ABaseWeapon* Weapon)
{
	float result = 1;

	if (GetPerkType() == Weapon->GetWeaponData().perk_type)
	{
		result *= GetPerkData().headshot_damage;
	}
	else
	{
		result *= GetPerkData().off_perk_headshot_damage;
	}

	return result;
}

float AKillingFloorLikeCharacter::GetReloadRate(ABaseWeapon* Weapon)
{
	float result = 1;

	if (GetPerkType() == Weapon->GetWeaponData().perk_type)
	{
		//예외처리 필요
		result *= PlayerPerkData.reload_rate;
	}

	return result;
}

float AKillingFloorLikeCharacter::GetRecoilMultiplier(ABaseWeapon* Weapon)
{
	float result = 1;

	if (GetPerkType() == Weapon->GetWeaponData().perk_type)
	{
		//TODO 예외처리?
		result *= GetPerkData().recoil;
	}

	return result;
}

float AKillingFloorLikeCharacter::GetDiscountRate(EPerkType WeaponPerkType)
{
	float result = 1;

	if (GetPerkType() == WeaponPerkType)
	{
		//TODO 예외처리?
		result *= GetPerkData().discount;
	}

	return result;
}

bool AKillingFloorLikeCharacter::IsBuyable(const FWeaponData& WeaponData)
{
	if (GetWeapon(WeaponData.type) != nullptr)
	{
		return false;
	}

	if (Weight + WeaponData.weight > MaxWeight)
	{
		return false;
	}

	if (Money < GetWeaponCost(WeaponData))
	{
		return false;
	}

	return true;
}

void AKillingFloorLikeCharacter::CalculateWeight()
{
	int32 WeightSum = 0;
	for (auto Element : WeaponInventory)
	{
		WeightSum += Element->GetWeaponData().weight;
	}
	//Weight = WeightSum;
	SetWeight(WeightSum);
}

float AKillingFloorLikeCharacter::GetMaxArmor()
{
	return MaxArmor;
}

float AKillingFloorLikeCharacter::GetCurrentArmor()
{
	return CurrentArmor;
}

void AKillingFloorLikeCharacter::Server_ChangePerk_Implementation(EPerkType Type, int32 Rank)
{
	if (GetIsChangablePerk() == false)
	{
		return;
	}
	SetPlayerPerkData(*GetKFLikeGameInstance()->GetPerkData(Type, Rank));

	IsChangablePerk = false;
}

/*void AKillingFloorLikeCharacter::Server_BuyWeaponFill_Implementation(ABaseWeapon* Weapon)
{
	// 1) 서버 권한 및 유효성 검사
	if (!HasAuthority() || Weapon == nullptr)
	{
		return;
	}
	// 무기 소유자 확인, 상점 이용 가능 여부 등 추가 검증
	// if (Weapon->GetOwner() != this || !bCanUseShop) return;

	// 2) 타입 캐스팅 및 파라미터 방어
	AConsumableWeapon* ConsumableWeapon = Cast<AConsumableWeapon>(Weapon);
	if (ConsumableWeapon == nullptr)
	{
		return;
	}

	// 3) 서버에서 비용 재계산(치트 방지)
	//    - 탄창 용량/가격/현재 돈/남은 탄약을 바탕으로
	//    - "탄창 단위" 구매 비용과 가능한 수량 계산
	const int32 MagCap = ConsumableWeapon->GetMagazineCapacity();
	const int32 MagCost = ConsumableWeapon->GetAmmoMagCost();
	const int32 MaxAmmo = ConsumableWeapon->GetAmmoCapacity();

	if (MagCap <= 0 || MagCost <= 0 || MaxAmmo <= 0)
	{
		return;
	}

	const int32 MyMoney = GetMoney();
	const int32 Missing = FMath::Max(0, MaxAmmo - ConsumableWeapon->GetSavedAmmo()); // 접근자/세터는 실제 코드에 맞게
	if (Missing <= 0 || MyMoney < MagCost)
	{
		return; // 이미 가득이거나 최소 1탄창 비용도 없음
	}

	// 필요한 탄창 수(올림), 살 수 있는 탄창 수(내림) → 실제 구매 탄창 수
	const int32 MagsNeeded = FMath::CeilToInt(static_cast<float>(Missing) / MagCap);
	const int32 MagsAffordable = MyMoney / MagCost;
	const int32 MagsToBuy = FMath::Clamp(MagsAffordable, 0, MagsNeeded);
	if (MagsToBuy <= 0)
	{
		return;
	}

	const int32 Cost = MagsToBuy * MagCost;
	const int32 AmmoToAdd = MagsToBuy * MagCap;

	// 4) 돈 차감 → 탄약 추가(용량 클램프) → 복제/알림
	SetMoney(GetMoney() - Cost); // 서버에서 권위 있게 차감
	ConsumableWeapon->AddAmmo(AmmoToAdd); // 내부에서 용량 클램프 보장

	// 필요 시 UI 갱신을 위한 알림/브로드캐스트
	// Publish/OnRep 호출로 클라에 반영되도록
}*/


void AKillingFloorLikeCharacter::Server_BuyWeaponMag_Implementation(ABaseWeapon* Weapon)
{
	if (Weapon == nullptr)
	{
		return;
	}
	AConsumableWeapon* ConsumableWeapon = Cast<AConsumableWeapon>(Weapon);
	if (ConsumableWeapon == nullptr)
	{
		return;
	}

	if (ConsumableWeapon->GetSavedAmmo() >= ConsumableWeapon->GetAmmoCapacity())
	{
		return;
	}

	if (ConsumableWeapon->GetAmmoMagCost() > GetMoney())
	{
		return;
	}

	SetMoney(GetMoney() - ConsumableWeapon->GetAmmoMagCost());

	ConsumableWeapon->AddAmmo(ConsumableWeapon->GetWeaponData().magazine_capacity);
}

void AKillingFloorLikeCharacter::Server_BuyArmor_Implementation()
{
	if (HasAuthority() == false)
	{
		return;
	}
	if (GetMoney() < ArmorCost)
	{
		return;
	}
	if (GetCurrentArmor() >= GetMaxArmor())
	{
		return;
	}

	SetMoney(GetMoney() - ArmorCost);

	SetCurrentArmor(GetMaxArmor());
}

UKFLikeGameInstance* AKillingFloorLikeCharacter::GetKFLikeGameInstance()
{
	if (GameInstance == nullptr)
	{
		GameInstance = Cast<UKFLikeGameInstance>(GetGameInstance());
	}
	return GameInstance;
}

/*void AKillingFloorLikeCharacter::OnRep_WeaponInventory()
{
	CalculateWeight();
}*/

void AKillingFloorLikeCharacter::CheckAllWeaponsReadyAndProcess()
{
	bool bAllWeaponsValid = true;
	for (ABaseWeapon* Weapon : WeaponInventory)
	{
		if (Weapon == nullptr) // 아직 NULL인 포인터가 있다면
		{
			bAllWeaponsValid = false;
			break;
		}
	}

	if (bAllWeaponsValid)
	{
		// 모든 무기 액터가 유효하게 복제되어 준비 완료!
		UE_LOG(LogTemp, Log, TEXT("All weapons in WeaponMap are now valid on %s!"),
		       HasAuthority() ? TEXT("Server") : TEXT("Client"));

		// 지연 타이머가 있다면 클리어
		GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CheckWeaponsReady);

		// 여기에 무기 관련 최종 로직을 수행합니다 (예: UI 업데이트, 초기 무기 장착 등).
		// OnAllWeaponsReady.Broadcast(); // 블루프린트에서 수신할 수 있도록 델리게이트 브로드캐스트
	}
	else
	{
		// 아직 NULL인 포인터가 있다면, 잠시 후 다시 확인하도록 타이머 설정
		UE_LOG(LogTemp, Warning, TEXT("Not all weapons valid yet. Retrying check in 0.1 seconds..."));
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_CheckWeaponsReady, this,
		                                       &AKillingFloorLikeCharacter::CheckAllWeaponsReadyAndProcess, 0.1f,
		                                       false);
	}
}

void AKillingFloorLikeCharacter::OnRep_CurrentWeapon()
{
	//애니메이션 실행
	if (HasActorBegunPlay() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("HasActorBegunPlay is False!!"));
		PlayerAnimInstance = Cast<UPlayerAnimInstance>(GetMesh()->GetAnimInstance());
		FirstMeshLocation = GetMesh1P()->GetRelativeLocation();

		SetCurrentArmor(100);

		SetPlayerPerkData(*GetKFLikeGameInstance()->GetPerkData(EPerkType::Sharpshooter, 6));
	}
	SwapWeaponCallback(GetCurrentWeapon());
}

void AKillingFloorLikeCharacter::OnRep_Money()
{
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Player_MoneyChange, this,
	                                                           FGameplayMessage_PlayerMoneyChange(this, GetMoney()));
}

void AKillingFloorLikeCharacter::OnRep_Weight()
{
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Player_WeaponChange, this,
	                                                           FGameplayMessage_None(this));
}

void AKillingFloorLikeCharacter::OnRep_PlayerPerkData()
{
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Player_PerkChange, this,
	                                                           FGameplayMessage_Perk(this, PlayerPerkData));
}

int32 AKillingFloorLikeCharacter::GetWeaponCost(const FWeaponData& WeaponData)
{
	return WeaponData.price * GetDiscountRate(WeaponData.perk_type);
}


void AKillingFloorLikeCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D

	FVector2D MovementVector = Value.Get<FVector2D>();

	if (IsControllable() == false)
	{
		return;
	}

	if (Controller != nullptr)
	{
		// add movement 
		AddMovementInput(GetActorForwardVector(), MovementVector.Y);
		AddMovementInput(GetActorRightVector(), MovementVector.X);
	}
}

void AKillingFloorLikeCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (IsControllable() == false)
	{
		return;
	}

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AKillingFloorLikeCharacter::Server_CheckChangableAimType_Implementation()
{
	ABaseWeapon* Weapon = GetCurrentWeapon();
	if (Weapon == nullptr || Weapon->IsAimTypeChangeable() == false)
	{
		return;
	}
	Multi_ChangeAimType();
}

void AKillingFloorLikeCharacter::Multi_ChangeAimType_Implementation()
{
	if (GetMesh1P() != nullptr && GetMesh1P()->GetAnimInstance() != nullptr)
	{
		GetMesh1P()->GetAnimInstance()->Montage_Stop(0.2f);
		GetMesh()->GetAnimInstance()->Montage_Stop(0.2f);
	}
	if (HasAuthority())
	{
		SetIsIron(!GetIsIron());
	}
}

void AKillingFloorLikeCharacter::Server_ReloadWeapon_Implementation()
{
	if (IsControllable() == false)
	{
		return;
	}

	ARangeWeapon* CurWeapon = Cast<ARangeWeapon>(GetCurrentWeapon());

	if (CurWeapon == nullptr || CurWeapon->IsReloadable() == false)
	{
		return;
	}

	UAnimMontage* ReloadMontage = CurWeapon->GetWeaponMontage(EWeaponAnimationType::Reload);
	UAnimMontage* PlayerReloadMontage = CurWeapon->GetPlayerAnimation(EWeaponAnimationType::Reload);
	Server_CheckPlayWeaponAnim(ReloadMontage, PlayerReloadMontage, false, GetReloadRate(CurWeapon));

	SetIsIron(false);
}

void AKillingFloorLikeCharacter::Multi_ReloadWeaponCallback_Implementation()
{
	ABaseWeapon* CurWeapon = GetCurrentWeapon();

	if (CurWeapon == nullptr)
	{
		return;
	}
	
	UAnimMontage* ReloadMontage = CurWeapon->GetWeaponMontage(EWeaponAnimationType::Reload);
	if (ReloadMontage == nullptr)
	{
		return;
	}
	UAnimInstance* AnimInstance = GetMesh1P()->GetAnimInstance();
	if (AnimInstance == nullptr || AnimInstance->Montage_IsPlaying(ReloadMontage))
	{
		return;
	}

	AnimInstance->Montage_Play(ReloadMontage, GetReloadRate(GetCurrentWeapon()));

	UAnimMontage* PlayerReloadMontage = CurWeapon->GetPlayerAnimation(EWeaponAnimationType::Reload);
	if (PlayerReloadMontage == nullptr)
	{
		return;
	}

	if (PlayerAnimInstance == nullptr || PlayerAnimInstance->Montage_IsPlaying(PlayerReloadMontage))
	{
		return;
	}

	PlayerAnimInstance->Montage_Play(PlayerReloadMontage, GetReloadRate(GetCurrentWeapon()));
}

void AKillingFloorLikeCharacter::DropWeapon()
{
	Server_DropWeapon(GetCurrentWeapon());
}

void AKillingFloorLikeCharacter::Server_DropWeapon_Implementation(ABaseWeapon* DropWeapon)
{
	if (IsControllable() == false)
	{
		return;
	}

	if (DropWeapon == nullptr)
	{
		return;
	}

	//권총?
	if (DropWeapon->GetWeaponData().can_sell == false)
	{
		return;
	}

	DropWeapon->DropWeapon(this);

	RemoveWeaponInventory(DropWeapon);


	SetCurrentWeapon(GetBestWeaponToEquip());
}

void AKillingFloorLikeCharacter::Server_SellWeapon_Implementation(EWeaponType WeaponType)
{
	if (GetWeapon(WeaponType) == nullptr)
	{
		return;
	}

	if (GetWeapon(WeaponType)->GetWeaponData().can_sell == false)
	{
		return;
	}

	ABaseWeapon* weapon = GetWeapon(WeaponType);
	SetMoney(GetMoney() + weapon->GetSellCost());


	RemoveWeaponInventory(weapon);
	weapon->Destroy();

	// 찾은 최상위 무기(또는 인벤토리가 비었다면 nullptr)를 현재 무기로 설정합니다.
	SetCurrentWeapon(GetBestWeaponToEquip());
}

void AKillingFloorLikeCharacter::Server_UseGrenade_Implementation()
{
	ABaseWeapon* GrenadeWeapon = GetWeapon(EWeaponType::Grenade);
	if (GrenadeWeapon == nullptr)
	{
		return;
	}

	if (GrenadeWeapon->IsAttackable() == false)
	{
		return;
	}

	if (GetMesh1P()->GetAnimInstance()->Montage_IsPlaying(
			GrenadeWeapon->GetAnimation(EWeaponAnimationType::Frag, false, 0)) ||
		GetMesh1P()->GetAnimInstance()->Montage_IsPlaying(
			GetCurrentWeapon()->GetAnimation(EWeaponAnimationType::Select, false, 0)) ||
		GetMesh1P()->GetAnimInstance()->Montage_IsPlaying(
			GetCurrentWeapon()->GetAnimation(EWeaponAnimationType::PutDown, false, 0)))
	{
		return;
	}

	Server_SwapWeapon(GrenadeWeapon);
}

ABaseWeapon* AKillingFloorLikeCharacter::GetBestWeaponToEquip()
{
	// 인벤토리에서 가장 등급이 높은 무기를 찾아 장착합니다.
	ABaseWeapon* BestWeaponToEquip = nullptr;
	int32 HighestTier = 1000; // EWeaponType::None이 0이므로 -1로 시작하면 안전합니다.

	for (ABaseWeapon* Weapon : WeaponInventory)
	{
		if (Weapon->GetWeaponType() == EWeaponType::Grenade)
		{
			continue;
		}
		if (Weapon) // 유효성 검사
		{
			const int32 CurrentTier = static_cast<int32>(Weapon->GetWeaponType());
			if (CurrentTier < HighestTier)
			{
				HighestTier = CurrentTier;
				BestWeaponToEquip = Weapon;
			}
		}
	}

	return BestWeaponToEquip;
}


void AKillingFloorLikeCharacter::Server_PurchaseWeapon_Implementation(int32 WeaponId)
{
	const FWeaponData* WeaponData = GetKFLikeGameInstance()->GetWeaponData(WeaponId);
	if (WeaponData == nullptr || IsBuyable(*WeaponData) == false)
	{
		return;
	}

	//SubtractMoney(GetWeaponCost(*WeaponData));
	SetMoney(GetMoney() - GetWeaponCost(*WeaponData));
	GetNewWeapon(WeaponId);
}

void AKillingFloorLikeCharacter::GetNewWeapon(int32 WeaponId)
{
	// 스폰 위치와 회전 설정
	FVector SpawnLocation = GetActorLocation(); // 또는 원하는 위치
	FRotator SpawnRotation = GetActorRotation(); // 또는 원하는 회전

	// 스폰 파라미터 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	// 무기 스폰
	ABaseWeapon* SpawnedWeapon = GetKFLikeGameInstance()->GetSubsystem<UObjectPoolManager>()->GetFromPoolTemplate<
		ABaseWeapon>(
		GetWorld(), GetKFLikeGameInstance()->GetWeaponClass(WeaponId), SpawnLocation, SpawnRotation, SpawnParams);

	SpawnedWeapon->AttachWeaponToCharacter(this);
}

void AKillingFloorLikeCharacter::OnFireStarted()
{
	Server_FireWeapon(ETriggerEvent::Started);
}

void AKillingFloorLikeCharacter::OnFireTriggered()
{
	Server_FireWeapon(ETriggerEvent::Triggered);
}

void AKillingFloorLikeCharacter::Server_FireWeapon_Implementation(ETriggerEvent TriggerEvent)
{
	// 발사 요청을 현재 무기에 위임합니다.
	// 무기는 발사 가능 여부, 애니메이션, 쿨다운 설정 등 모든 관련 로직을 스스로 처리합니다.
	if (ABaseWeapon* CurWeapon = GetCurrentWeapon())
	{
		CurWeapon->Server_RequestFire(TriggerEvent);
	}
}

void AKillingFloorLikeCharacter::Server_SpecialFireWeapon_Implementation()
{
	if (IsControllable() == false)
	{
		return;
	}
	ABaseWeapon* CurWeapon = GetCurrentWeapon();
	IWeaponSkillInterface* inter = Cast<IWeaponSkillInterface>(CurWeapon);
	if (CurWeapon->IsAttackable() == false || inter == nullptr)
	{
		return;
	}

	Server_CheckPlayWeaponAnim(CurWeapon->GetWeaponMontage(EWeaponAnimationType::SpecialFire),
	                           CurWeapon->GetPlayerAnimation(EWeaponAnimationType::SpecialFire), true);

	CurWeapon->SetCurrentAttackCooltime();
}

void AKillingFloorLikeCharacter::GetSwapWeaponInputValue(const FInputActionValue& ActionValue)
{
	Server_SwapWeaponCheck(ActionValue.Get<float>());
}

void AKillingFloorLikeCharacter::Server_SwapWeaponCheck_Implementation(int32 ActionValue)
{
	EWeaponType SwapWeaponType = IsSwapWeaponable(ActionValue);
	if (SwapWeaponType == EWeaponType::None)
	{
		return;
	}

	ABaseWeapon* CurWeapon = GetCurrentWeapon();
	if (CurWeapon == nullptr)
	{
		return;
	}

	UAnimMontage* AnimMontage = CurWeapon->GetWeaponMontage(EWeaponAnimationType::PutDown);
	if (AnimMontage == nullptr)
	{
		return;
	}


	UAnimInstance* AnimInstance = GetMesh1P()->GetAnimInstance();
	if (AnimInstance->Montage_IsPlaying(AnimMontage))
	{
		return;
	}

	Server_SwapWeapon(GetWeapon(SwapWeaponType));
}

EWeaponType AKillingFloorLikeCharacter::IsSwapWeaponable(int32 ActionValue)
{
	if (IsControllable() == false)
	{
		return EWeaponType::None;
	}

	if (ActionValue == 1 && GetWeapon(EWeaponType::Main) != nullptr && CurrentWeapon->GetWeaponType() !=
		EWeaponType::Main)
	{
		return EWeaponType::Main;
	}
	else if (ActionValue == 2 && GetWeapon(EWeaponType::Sub) != nullptr && CurrentWeapon->GetWeaponType() !=
		EWeaponType::Sub)
	{
		return EWeaponType::Sub;
	}
	else if (ActionValue == 3 && GetWeapon(EWeaponType::Knife) != nullptr && CurrentWeapon->GetWeaponType() !=
		EWeaponType::Knife)
	{
		return EWeaponType::Knife;
	}
	else if (ActionValue == 4 && GetWeapon(EWeaponType::Syringe) != nullptr && CurrentWeapon->GetWeaponType() !=
		EWeaponType::Syringe)
	{
		return EWeaponType::Syringe;
	}

	return EWeaponType::None;
}

//multi로 실행됨
//클라1이 무기를 변경하면 클라 2는 아무것도 안함.
//
void AKillingFloorLikeCharacter::Server_SwapWeapon_Implementation(ABaseWeapon* NewWeapon)
{
	ABaseWeapon* CurWeapon = GetCurrentWeapon();
	if (CurWeapon == nullptr)
	{
		return;
	}
	
	UAnimMontage* AnimMontage = CurWeapon->GetWeaponMontage(EWeaponAnimationType::PutDown);
	if (AnimMontage == nullptr)
	{
		return;
	}

	UAnimInstance* AnimInstance = GetMesh1P()->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return;
	}

	SetIsIron(false);

	Multi_PlayPutDownAnimation(NewWeapon, AnimMontage);
}

void AKillingFloorLikeCharacter::Multi_PlayPutDownAnimation_Implementation(
	ABaseWeapon* NewWeapon, UAnimMontage* AnimMontage)
{
	UAnimInstance* AnimInstance = GetMesh1P()->GetAnimInstance();
	if (AnimInstance == nullptr)
	{
		return;
	}

	AnimInstance->Montage_Play(AnimMontage);

	if (HasAuthority())
	{
		FOnMontageEnded BlendOutDelegate;
		BlendOutDelegate.BindLambda([this, NewWeapon](UAnimMontage* PlayedMontage, bool bInterrupted)
		{
			OnPutDownMontageEnded(bInterrupted, NewWeapon);
		});

		AnimInstance->Montage_SetEndDelegate(BlendOutDelegate, AnimMontage);
	}
}

void AKillingFloorLikeCharacter::OnPutDownMontageEnded(bool bInterrupted,
                                                       ABaseWeapon* NewWeapon)
{
	if (HasAuthority() == false)
	{
		return;
	}

	if (bInterrupted || NewWeapon == nullptr)
	{
		return;
	}

	Multi_OnPutDownMontageEndedCallback();

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this,NewWeapon]()
	{
		if (NewWeapon->GetWeaponType() != EWeaponType::Grenade)
		{
			SetCurrentWeapon(NewWeapon);
		}
		else
		{
			Multi_SwapWeaponCallback(NewWeapon);
		}
	});

	GetWorldTimerManager().SetTimerForNextTick(TimerDelegate);
}

void AKillingFloorLikeCharacter::Multi_OnPutDownMontageEndedCallback_Implementation()
{
	if (GetCurrentWeapon() == nullptr)
	{
		return;
	}
	UWeaponFleshComponent* FleshComponent = GetCurrentWeapon()->GetComponentByClass<UWeaponFleshComponent>();
	if (FleshComponent)
	{
		FleshComponent->ToggleFlashlight(false);
	}
	GetMesh1P()->SetVisibility(false);

	if (IsLocallyControlled() == false)
	{
		GetCurrentWeapon()->GetStaticMeshComponent()->SetVisibility(false);
	}
}

void AKillingFloorLikeCharacter::Server_CheckPlayWeaponAnim_Implementation(
	UAnimMontage* WeaponMontage, UAnimMontage* PlayerMontage, bool IsStopPlayingMontage, float PlayRate)
{
	// 1. 유효성 검사: 재생할 애니메이션이 하나도 없으면 아무것도 하지 않습니다.
	if (WeaponMontage == nullptr && PlayerMontage == nullptr)
	{
		return;
	}

	// 2. 1인칭 메쉬의 AnimInstance를 가져옵니다.
	UAnimInstance* AnimInstance1P = GetMesh1P()->GetAnimInstance();
	if (!IsValid(AnimInstance1P))
	{
		return;
	}
    
	// 3. 이미 재생 중인 경우의 처리
	const bool bIsPlaying = AnimInstance1P->Montage_IsPlaying(WeaponMontage);
	if (bIsPlaying)
	{
		// 3a. 이미 재생 중인데, 중단하라는 요청이 아니면 그냥 무시합니다.
		if (!IsStopPlayingMontage)
		{
			return;
		}
		// 3b. 이미 재생 중이고, 중단 요청이 있으면 몽타주를 멈춥니다.
		AnimInstance1P->Montage_Stop(0.1f, WeaponMontage);
	}
    
	// 4. 모든 클라이언트에게 몽타주를 재생하라고 명령합니다.
	Multi_PlayWeaponAnim(WeaponMontage, PlayerMontage, PlayRate);

	// 5. 서버에만 필요한 특별 로직 (수류탄 애니메이션 종료 후 무기 교체)
	// 멀티캐스트가 아닌 서버 RPC에 위치해야 올바르게 동작합니다.
	if (WeaponMontage && GetWeapon(EWeaponType::Grenade) &&
		WeaponMontage == GetWeapon(EWeaponType::Grenade)->GetAnimation(EWeaponAnimationType::Frag, false, 0))
	{
		FOnMontageEnded BlendOutDelegate;
		BlendOutDelegate.BindUObject(this, &AKillingFloorLikeCharacter::OnGrenadeAnimationEnded);
		AnimInstance1P->Montage_SetEndDelegate(BlendOutDelegate, WeaponMontage);
	}
}


void AKillingFloorLikeCharacter::Multi_PlayWeaponAnim_Implementation(UAnimMontage* SelectMontage,
                                                                     UAnimMontage* PlayerMontage, float PlayRate)
{
	// 1. 3인칭 플레이어 메쉬 애니메이션을 재생합니다.
	if (IsValid(PlayerAnimInstance) && IsValid(PlayerMontage))
	{
		PlayerAnimInstance->Montage_Play(PlayerMontage, PlayRate);
	}

	// 2. 1인칭 무기 메쉬 애니메이션을 재생합니다.
	UAnimInstance* AnimInstance1P = GetMesh1P()->GetAnimInstance();
	if (IsValid(AnimInstance1P) && IsValid(SelectMontage))
	{
		AnimInstance1P->Montage_Play(SelectMontage, PlayRate);
	}

	// 3. 로컬 플레이어에게만 필요한 시각적 처리 (1인칭 메쉬 깜빡임 방지)
	if (IsLocallyControlled())
	{
		// 다음 틱에 Mesh1P를 다시 보이게 하여 깜빡임 현상을 방지합니다.
		GetWorldTimerManager().SetTimerForNextTick([this]()
		{
			if (GetMesh1P())
			{
				GetMesh1P()->SetVisibility(true);
			}
		});
	}
}

void AKillingFloorLikeCharacter::OnGrenadeAnimationEnded(UAnimMontage* PlayedMontage, bool bInterrupted)
{
    // 애니메이션이 중간에 끊겼다면 아무것도 하지 않습니다.
    if (bInterrupted)
    {
        return;
    }
    
    // 이전 무기로 교체하는 로직을 호출합니다.
    Multi_DelayedSwapWeaponCallback(GetCurrentWeapon());
}

void AKillingFloorLikeCharacter::SetHasRifle(bool bNewHasRifle)
{
	bHasRifle = bNewHasRifle;
}

bool AKillingFloorLikeCharacter::GetHasRifle()
{
	return bHasRifle;
}

void AKillingFloorLikeCharacter::Server_PickUpWeapon_Implementation(ABaseWeapon* NewWeapon)
{
	if (IsDead())
	{
		return;
	}

	if (NewWeapon == nullptr)
	{
		return;
	}

	if (GetWeapon(NewWeapon->GetWeaponType()))
	{
		return;
	}

	AddWeaponInventory(NewWeapon);

	//첫 무기 획득하면 그냥 바로 CurrentWeapon = NewWeapon
	if (GetCurrentWeapon() == nullptr)
	{
		SetCurrentWeapon(NewWeapon);
		return;
	}

	ABaseWeapon* NewCurrentWeapon = GetBestWeaponToEquip();
	// 우선순위가 높은 무기이면 교체 로직을 실행합니다.
	if (NewWeapon == NewCurrentWeapon)
	{
		SetCurrentWeapon(NewWeapon);
		//Server_SwapWeapon(NewWeapon);
	}
}

void AKillingFloorLikeCharacter::SwapWeaponCallback(ABaseWeapon* Weapon)
{
	if (Weapon == nullptr)
	{
		return;
	}

	// 새로운 Skeletal Mesh와 AnimInstance 설정
	Mesh1P->SetSkeletalMesh(Weapon->GetSkeletalMesh(), true);
	Mesh1P->SetAnimInstanceClass(Weapon->GetAnimInstance());
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Player_AmmoChange, this,
	                                                           FGameplayMessage_PlayerAmmoChange(this, Weapon));

	//TODO 임시
	if (Weapon->GetWeaponType() != EWeaponType::Grenade)
	{
		PlayerAnimInstance->CurrentLocomotionBS = GetGameInstance()->GetSubsystem<UResourceManager>()->
		                                                             LoadPlayerBlendSpace(
			                                                             CurrentWeapon->GetWeaponData().id, false);
	}

	if (IsLocallyControlled() == false)
	{
		Weapon->GetStaticMeshComponent()->SetVisibility(true);
	}

	if (IsLocallyControlled())
	{
		if (Weapon->GetWeaponType() != EWeaponType::Grenade)
		{
			Server_CheckPlayWeaponAnim(Weapon->GetAnimation(EWeaponAnimationType::Select, GetIsIron(), 0), nullptr);
		}
		else
		{
			Server_CheckPlayWeaponAnim(Weapon->GetAnimation(EWeaponAnimationType::Frag, false, 0), nullptr);
		}
		//select할 때 한틱 후 보여줘야 자연스러움 
	}
}

void AKillingFloorLikeCharacter::Multi_DelayedSwapWeaponCallback_Implementation(ABaseWeapon* Weapon)
{
	GetMesh1P()->SetVisibility(false);
	// DEFERRED EXECUTION: 이 콜백은 애니메이션 시스템의 틱 내부에서 실행됩니다.
	// 여기서 SetSkeletalMesh를 직접 호출하면 재귀적인 평가로 인해 크래시가 발생합니다.
	// 타이머를 사용하여 다음 게임 틱으로 실행을 지연시켜야 합니다.
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this, Weapon]()
	{
		SwapWeaponCallback(Weapon);
	});
	GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
}

void AKillingFloorLikeCharacter::Multi_SwapWeaponCallback_Implementation(ABaseWeapon* Weapon)
{
	SwapWeaponCallback(Weapon);
}

void AKillingFloorLikeCharacter::Server_DropMoney_Implementation()
{
	if (IsControllable() == false)
	{
		return;
	}

	if (GetMoney() <= 0)
	{
		return;
	}

	FActorSpawnParameters ActorSpawnParams;
	ActorSpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	TSubclassOf<AActor> actor = *GetKFLikeGameInstance()->GetGeneralObjectClass(EGenetalObjectType::Money);

	AMoneyObject* MoneyObject = GetKFLikeGameInstance()->GetSubsystem<UObjectPoolManager>()->GetFromPoolTemplate<
		AMoneyObject>(
		GetWorld(), actor, GetActorLocation() + GetActorForwardVector() * 100, GetActorRotation(), ActorSpawnParams);

	if (MoneyObject)
	{
		UStaticMeshComponent* x = MoneyObject->GetComponentByClass<UStaticMeshComponent>();
		x->AddImpulse(GetActorForwardVector() * 5000);

		int32 Amount = GetMoney() > 50 ? 50 : GetMoney();
		SetMoney(GetMoney() - Amount);
		MoneyObject->SetMoneyAmount(Amount);
		if (UTP_PickUpComponent* PickUpComponent = MoneyObject->FindComponentByClass<UTP_PickUpComponent>())
		{
			PickUpComponent->OnDrop.Broadcast(this);
		}
	}
}

void AKillingFloorLikeCharacter::Client_PlayHitEffect_Implementation(FDamageEvent const& DamageEvent, float NewArmor,
                                                                     float NewHp)
{
	UE_LOG(LogTemp, Display, TEXT("Client_PlayHitEffect : %f, %f"), NewArmor, NewHp);
	if (IsLocallyControlled())
	{
		PlayerCharacterController->GetKillingFloorHud()->GetWbCrossHairInstance()->Client_PlayHitAnimation(
			DamageEvent.DamageTypeClass, NewHp);
	}
}

void AKillingFloorLikeCharacter::OnRep_IsIron()
{
	if (IsLocallyControlled() && GetMesh1P() != nullptr && GetMesh1P()->GetAnimInstance() != nullptr)
	{
		EventChangeFov(GetIsIron());
	}
}

void AKillingFloorLikeCharacter::EndGameCallback(FGameplayTag Channel, const FGameplayMessage_EndMatch& Message)
{
	//Server_ChangeUnitState(EUnitState::UnMovable, true);
}

void AKillingFloorLikeCharacter::Server_RequestShoot_Implementation()
{
	// 이 함수의 모든 책임은 ABaseWeapon::RequestFire로 이전되었습니다.
	// 이 함수는 더 이상 필요하지 않으므로, 호출하는 곳이 없다면 제거하는 것을 고려할 수 있습니다.
	// 만약 다른 곳에서 호출한다면, 해당 호출을 Server_FireWeapon으로 변경해야 합니다.
}

void AKillingFloorLikeCharacter::Multi_RestartCallback_Implementation()
{
	if (HasAuthority())
	{
		Server_ChangeUnitState(EUnitState::Idle, true);
		SetCurrentHp(100);
	}
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));

	GetMesh()->SetCollisionProfileName(TEXT("Player"));
	GetMesh()->SetSimulatePhysics(false);

	if (UCharacterMovementComponent* MoveComp = GetCharacterMovement())
	{
		MoveComp->SetMovementMode(MOVE_Walking);
	}
}
