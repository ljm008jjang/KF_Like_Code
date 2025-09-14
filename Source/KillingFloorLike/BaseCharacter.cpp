// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "HealDamageType.h"
#include "KillingFloorLikeGameMode.h"
#include "SoundManager.h"
#include "Components/AudioComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	GetCharacterMovement()->SetIsReplicated(true);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(GetMesh()); // 캐릭터의 메시에 붙입니다.
	AudioComponent->bAutoActivate = false; // 자동으로 소리가 재생되지 않도록 설정합니다.

	// 2. 이 컴포넌트가 리플리케이트 될 것이라고 명시합니다. (가장 중요!)
	AudioComponent->SetIsReplicated(true);
}

void ABaseCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// 아래 변수들을 네트워크를 통해 복제하도록 등록합니다.
	DOREPLIFETIME(ABaseCharacter, CurrentHp);
	DOREPLIFETIME(ABaseCharacter, CurrentUnitState);
	DOREPLIFETIME(ABaseCharacter, CurrentUnitType);
	//DOREPLIFETIME(ABaseCharacter, AudioComponent);
}


// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	SetCurrentHp(MaxHp);
	if (HasAuthority())
	{
		Server_ChangeUnitState(EUnitState::Idle);
	}
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	SetReplicateMovement(true);

	/*if (Controller)
	{
		// 컨트롤러의 이름을 출력합니다. "PlayerController_0" 같은 이름이 보인다면 플레이어가 빙의한 것입니다.
		// AI가 빙의했다면 "AIController_0" 같은 이름이 보일 것입니다.
		UE_LOG(LogTemp, Warning, TEXT("%hhd, %s is possessed by: %s"), HasAuthority(), *GetName(),
		       *Controller->GetName());
		UE_LOG(LogTemp, Warning, TEXT("%hhd, is IsLocallyControlled: %hhd"), HasAuthority(), IsLocallyControlled());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("%hhd, %s has no controller yet."), HasAuthority(), *GetName());
		UE_LOG(LogTemp, Warning, TEXT("%hhd, is IsLocallyControlled: %hhd"), HasAuthority(), IsLocallyControlled());
	}*/

	//이상하게 monster도 서버에서 IsLocallyControlled라고 함.
	/*if (IsLocallyControlled())
	{
		GetMesh()->SetVisibility(false);
		GetMesh()->SetHiddenInGame(true);
	}*/
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void ABaseCharacter::OnRep_UnitState()
{
	if (CurrentUnitState == EUnitState::Walk)
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
	else if (CurrentUnitState == EUnitState::Run)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
	else if (CurrentUnitState == EUnitState::Dead)
	{
		if (HasAuthority())
		{
			GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Player_Dead,this,
			                                                           FGameplayMessage_None());
		}
		if (Controller)
		{
			Controller->StopMovement();
		}
	}
	else if (CurrentUnitState == EUnitState::UnMovable)
	{
		if (Controller)
		{
			Controller->StopMovement();
		}
	}
}

void ABaseCharacter::OnRep_CurrentHealth()
{
}

void ABaseCharacter::Attack()
{
	if (IsControllable() == false)
	{
		return;
	}

	// 월드에서 현재 위치를 기준으로 검색할 위치와 반경 정의
	FVector SearchOrigin = GetActorLocation(); // 검색 중심점
	float SearchRadius = 50.0f; // 검색 반경

	// 검색 조건 설정
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes; // 검색할 객체 타입

	TArray<AActor*> IgnoredActors; // 검색에서 제외할 액터 리스트
	TArray<AActor*> OutActors; // 검색 결과를 담을 배열

	// SphereOverlapActors 호출
	bool bFound = UKismetSystemLibrary::SphereOverlapActors(
		GetWorld(),
		SearchOrigin,
		SearchRadius,
		ObjectTypes,
		ABaseCharacter::StaticClass(),
		IgnoredActors,
		OutActors
	);
	bool IsHitted = false;

	// 결과 출력
	if (bFound)
	{
		for (AActor* FoundActor : OutActors)
		{
			if (FoundActor && FoundActor != this)
			{
				ABaseCharacter* FoundBaseCharacter = Cast<ABaseCharacter>(FoundActor);
				if (FoundBaseCharacter && IsAttackableUnitType(FoundBaseCharacter))
				{
					UGameplayStatics::ApplyDamage(
						FoundBaseCharacter,
						BaseAttackDamage,
						GetInstigatorController(),
						this,
						UDamageType::StaticClass()
					);
					UE_LOG(LogTemp, Log, TEXT("%s"), *FoundBaseCharacter -> GetName());
					IsHitted = true;
				}
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("No actors found in radius."));
	}
	if (IsHitted)
	{
		Multi_PlaySoundBase(EMonsterSoundType::AttackEnemy);
	}
}

bool ABaseCharacter::IsControllable()
{
	return GetCurrentUnitState() != EUnitState::UnMovable && GetCurrentUnitState() != EUnitState::Dead;
}

bool ABaseCharacter::IsDead()
{
	return GetCurrentUnitState() == EUnitState::Dead;
}


void ABaseCharacter::OnRep_AudioComponent()
{
	//if ()
}

void ABaseCharacter::Dead(AActor* DamageCauser)
{
	if (HasAuthority() == false)
	{
		return;
	}
	if (GetIsAlive() == false)
	{
		return;
	}

	Server_ChangeUnitState(EUnitState::Dead, true);

	Multi_DeadCallback(DamageCauser);
}

void ABaseCharacter::Multi_DeadCallback_Implementation(AActor* DamageCauser)
{
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("NoCollision"));

	GetMesh()->SetCollisionProfileName(TEXT("DeadActor"));
	GetMesh()->SetSimulatePhysics(true);

	if (GetMesh()->GetAnimInstance())
	{
		GetMesh()->GetAnimInstance()->StopAllMontages(0);
	}

	if (Controller)
	{
		Controller->StopMovement();
	}
}

float ABaseCharacter::CalcFinalDamageAmount(FDamageEvent const& DamageEvent, float amount)
{
	//힐이 아니면 음수처리
	if (DamageEvent.DamageTypeClass != UHealDamageType::StaticClass())
	{
		amount *= -1;
	}

	return amount;
}

float ABaseCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                                 AActor* DamageCauser)
{
	float realDamageAmount = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);


	SetCurrentHp(GetCurrentHP() + CalcFinalDamageAmount(DamageEvent, realDamageAmount));


	//UE_LOG(LogTemp, Display, TEXT("%f"), CurrentHp);

	if (CurrentHp <= 0)
	{
		Dead(DamageCauser);
		Multi_PlaySoundBase(EMonsterSoundType::Dead);
	}
	else
	{
		Multi_PlaySoundBase(EMonsterSoundType::Hit);
	}

	if (CurrentHp >= MaxHp)
	{
		SetCurrentHp(MaxHp);
	}

	return realDamageAmount;
}

float ABaseCharacter::GetMaxHP()
{
	return MaxHp;
}

float ABaseCharacter::GetCurrentHP()
{
	return CurrentHp;
}

float ABaseCharacter::GetHpRatio()
{
	return CurrentHp / MaxHp;
}

void ABaseCharacter::Server_ChangeUnitState_Implementation(EUnitState NewUnitState, bool IsHard)
{
	if (CurrentUnitState == NewUnitState)
	{
		return;
	}

	if (IsHard == false && CurrentUnitState == EUnitState::Dead)
	{
		return;
	}

	if (HasAuthority())
	{
		CurrentUnitState = NewUnitState;
		OnRep_UnitState();
	}

	/*if (CurrentUnitState == EUnitState::Walk)
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
	else if (CurrentUnitState == EUnitState::Run)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
	else if (CurrentUnitState == EUnitState::Dead)
	{
		Controller->StopMovement();
	}
	else if (CurrentUnitState == EUnitState::Dead)
	{
		Controller->StopMovement();
	}*/
}

/*void ABaseCharacter::ChangeUnitState(const EUnitState NewUnitState, const bool IsHard)
{
	if (CurrentUnitState == NewUnitState)
	{
		return;
	}

	if (IsHard == false && CurrentUnitState == EUnitState::Dead)
	{
		return;
	}

	CurrentUnitState = NewUnitState;

	if (CurrentUnitState == EUnitState::Walk)
	{
		GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
	}
	else if (CurrentUnitState == EUnitState::Run)
	{
		GetCharacterMovement()->MaxWalkSpeed = RunSpeed;
	}
	else if (CurrentUnitState == EUnitState::Dead)
	{
		if (HasAuthority())
		{
			AKillingFloorLikeGameMode* GameMode = Cast<
				AKillingFloorLikeGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
			GameMode->OnUpdateRemainMonsterCount();
		}
	}
}*/

/*void ABaseCharacter::HitEffect(const FHitResult& Hit)
{
	if (Hit.BoneName.ToString().Contains(TEXT("Head")))
	{
		GetMesh()->HideBoneByName(Hit.BoneName, PBO_None);
	}
}*/


EUnitState ABaseCharacter::GetCurrentUnitState()
{
	return CurrentUnitState;
}

bool ABaseCharacter::GetIsAlive()
{
	return GetCurrentUnitState() != EUnitState::Dead;
}

EKFUnitType ABaseCharacter::GetCurrentUnitType()
{
	return CurrentUnitType;
}

bool ABaseCharacter::IsAttackableUnitType(ABaseCharacter* AttackedUnit)
{
	if (CurrentUnitType == AttackedUnit->GetCurrentUnitType())
	{
		return false;
	}

	return true;
}

void ABaseCharacter::Multi_StopAudioComponent_Implementation()
{
	if (this->AudioComponent && this->AudioComponent->IsValidLowLevel())
	{
		// 2. 재생 중인 사운드를 명시적으로 멈춥니다.
		this->AudioComponent->Stop();
		// 3. 컴포넌트를 파괴 목록에 올려 리소스를 정리합니다.
		this->AudioComponent->DestroyComponent();
	}
}

void ABaseCharacter::Multicast_PlayAndSetSound_Implementation(USoundBase* Sound)
{
	// 이 함수는 서버와 모든 클라이언트에서 실행됩니다.

	// 1. AudioComponent가 유효한지 확인합니다.
	// 이 컴포넌트는 생성자에서 만들어지므로, 일반적으로 항상 유효해야 합니다.
	if (AudioComponent && AudioComponent->IsValidLowLevel())
	{
		// 2. 새로운 사운드를 재생하기 전에, 현재 재생 중인 소리가 있다면 멈춥니다.
		if (AudioComponent->IsPlaying())
		{
			AudioComponent->Stop();
		}

		// 3. 새로운 사운드 애셋을 설정하고 재생합니다.
		// Sound가 nullptr이면 아무 소리도 나지 않게 됩니다.
		AudioComponent->SetSound(Sound);
		AudioComponent->Play();
	}
}

/*USoundBase* ABaseCharacter::GetSoundBase(EMonsterSoundType SoundType)
{
	return SoundMap[SoundType];
}*/

void ABaseCharacter::Multi_PlaySoundBase_Implementation(EMonsterSoundType SoundType)
{
	if (SoundMap.Contains(SoundType) && SoundMap[SoundType])
	{
		GetGameInstance()->GetSubsystem<USoundManager>()->Multi_Play3DSound(SoundMap[SoundType], GetActorLocation());
		//UGameplayStatics::PlaySoundAtLocation(this, SoundMap[SoundType], GetActorLocation());
	}
}

void ABaseCharacter::Multi_PlayCharacterAnim_Implementation(UAnimMontage* SelectMontage, bool IsStopOtherMontage)
{
	if (GetMesh() == nullptr || GetMesh()->GetAnimInstance() == nullptr || SelectMontage == nullptr)
	{
		return;
	}

	if (GetMesh()->GetAnimInstance()->IsAnyMontagePlaying())
	{
		if (IsStopOtherMontage)
		{
			GetMesh()->GetAnimInstance()->StopAllMontages(0);
		}
		else
		{
			return;
		}
	}

	if (SelectMontage)
	{
		GetMesh()->GetAnimInstance()->Montage_Play(SelectMontage);
	}
}
