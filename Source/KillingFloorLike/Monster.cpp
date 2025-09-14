// Fill out your copyright notice in the Description page of Project Settings.


#include "Monster.h"

#include "AIController.h"
#include "BaseWeapon.h"
#include "BrainComponent.h"
#include "GoreParticle.h"
#include "KFLikeGameInstance.h"
#include "KillingFloorLikeCharacter.h"
#include "KillingFloorLikeGameMode.h"
#include "KillingFloorLikeGameState.h"
#include "MonsterAnimInstance.h"
#include "ObjectPoolManager.h"
#include "PoolableComponent.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "UnitManager.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/GameState.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AMonster::AMonster()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	CurrentUnitType = EKFUnitType::Enemy;

	PoolableComponent = CreateDefaultSubobject<UPoolableComponent>(TEXT("PoolableComponent"));
}

void AMonster::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AMonster, HeadBoneName);
}


void AMonster::OnPooledActivate()
{
	BaseAttackDamage = MonsterData.damage;
	MaxHp = MonsterData.health;
	SetCurrentHp(MonsterData.health);
	CurrentHeadHP = MonsterData.head_health;
	WalkSpeed = MonsterData.speed;
	RunSpeed = MonsterData.run_speed;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	GetCapsuleComponent()->SetSimulatePhysics(false);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionProfileName(TEXT("MonsterMesh"));
	/*
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("Pawn"));
	GetMesh()->SetCollisionProfileName(TEXT("MonsterMesh"));*/
	if (AIController && AIController->GetBrainComponent() && AIController->GetBlackboardComponent())
	{
		AIController->GetBrainComponent()->RestartLogic();
		AIController->GetBlackboardComponent()->SetValueAsBool("IsMoveable", true);
		//AIController->GetBlackboardComponent()->SetValueAsBool("IsSkillable", true);
		AIController->GetBlackboardComponent()->SetValueAsBool("IsPlayerVisible", false);
		GetMesh()->SetAnimInstanceClass(GetMesh()->GetAnimClass());
	}

	if (UMonsterAnimInstance* MonsterAnimInstance = Cast<UMonsterAnimInstance>(GetMesh()->GetAnimInstance()))
	{
		MonsterAnimInstance->IsHeadless = false;
	}


	if (HasAuthority())
	{
		Server_ChangeUnitState(EUnitState::Idle, true);
	}
}

//Should look BP_Clot
void AMonster::OnPooledReset()
{
	UE_LOG(LogTemp, Display, TEXT("PoolReset"));
	GetMesh()->SetSimulatePhysics(false);


	GetMesh()->SetRelativeRotation(FRotator(0, -90, 0));
	GetMesh()->SetRelativeLocation(FVector(0, 0, -90));
	GetMesh()->UnHideBoneByName(HeadBoneName);
}

// Called when the game starts or when spawned
void AMonster::BeginPlay()
{
	Super::BeginPlay();
	GameInstance = Cast<UKFLikeGameInstance>(GetGameInstance());

	this->MonsterData = *GameInstance->GetMonsterData(GetId());
	AIController = Cast<AAIController>(GetController());

	PoolableComponent->OnActivated.AddDynamic(this, &AMonster::OnPooledActivate);
	PoolableComponent->OnDeactivated.AddDynamic(this, &AMonster::OnPooledReset);
}

void AMonster::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	Super::EndPlay(EndPlayReason);
}


// Called every frame
void AMonster::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

float AMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
                           AActor* DamageCauser)
{
	float realDamageAmount = AActor::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	bool IsPlayHeadShotAnim = false;

	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);

		UE_LOG(LogTemp, Display, TEXT("Hited bone name : %s"), *PointDamageEvent->HitInfo.BoneName.ToString());

		if (CurrentHeadHP > 0 && PointDamageEvent->HitInfo.BoneName.ToString().Contains(TEXT("Head")))
		{
			HeadBoneName = PointDamageEvent->HitInfo.BoneName;
			realDamageAmount *= Cast<ABaseWeapon>(DamageCauser)->GetHeadMultiplier(); // 맞은 부위가 Head면, 데미지 1.1배.
			CurrentHeadHP -= realDamageAmount;
			UE_LOG(LogTemp, Display, TEXT("CurrentHeadHP : %f"), CurrentHeadHP);

			if (CurrentHeadHP <= 0)
			{
				FActorSpawnParameters ActorSpawnParams;
				ActorSpawnParams.SpawnCollisionHandlingOverride =
					ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				TSubclassOf<AActor> actor = *GameInstance->GetGeneralObjectClass(
					EGenetalObjectType::GoreBodyParticle);
				FVector HeadLocation = GetMesh()->GetBoneLocation(HeadBoneName);
				for (int32 i = 0; i < 3; i++)
				{
					float rand = FMath::FRand();
					FVector SpawnLocation = HeadLocation + FVector(rand);
					AGoreParticle* GoreObject = GetGameInstance()->GetSubsystem<UObjectPoolManager>()->
					                                               GetFromPoolTemplate<
						                                               AGoreParticle>(
						                                               GetWorld(), actor, SpawnLocation,
						                                               GetActorRotation(), ActorSpawnParams);

					if (GoreObject)
					{
						//GoreObject->Client_AddImpurse(HeadLocation);
						UStaticMeshComponent* x = GoreObject->GetComponentByClass<UStaticMeshComponent>();
						x->AddRadialImpulse(HeadLocation, 1, 50, RIF_Constant);
					}
				}
				Multicast_HideHeadBoneAndExplode(HeadLocation, HeadBoneName, DamageCauser);
				IsPlayHeadShotAnim = true;

				//3퍼센트 확률로 제드타임
				if (FMath::FRand() < 0.03f)
				{
					Cast<AKillingFloorLikeGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->
						StartZedTime(1.5f, 0.3f);
				}

				realDamageAmount = realDamageAmount + 0.25f * MaxHp; //머리가 터지면 추가 데미지
				//GetMesh()->HideBoneByName(HeadBoneName, PBO_None);


				//TODO HeadOff작업? 다시 등장 시 블루프린트 작업 필요
			}
		}
	}
	// RadialDamage 받기.
	else if (DamageEvent.IsOfType(FRadialDamageEvent::ClassID))
	{
		const FRadialDamageEvent* RadialDamageEvent = static_cast<const FRadialDamageEvent*>(&DamageEvent);
	}

	SetCurrentHp(GetCurrentHP() + CalcFinalDamageAmount(DamageEvent, realDamageAmount));

	//특정 데미지 이상은 stun 애니메이션 실행
	if (IsPlayHeadShotAnim == false)
	{
		if (realDamageAmount >= GetStunThreshold())
		{
			SetBBIsMoveable(false);
			FOnMontageEnded BlendOutDelegate;
			BlendOutDelegate.BindLambda([this](UAnimMontage* Montage, bool bInterrupted) { SetBBIsMoveable(true); }
			);

			UAnimMontage* AnimMontage = GetAnimationMontage(EMonsterAnimationType::Stun, 0);

			Multi_PlayCharacterAnim(AnimMontage, true);
			GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(BlendOutDelegate, AnimMontage);
		}
		//그 외는 hit 애니메이션 실행
		else
		{
			Multi_PlayCharacterAnim(GetHitAnimMontage(DamageCauser));
		}
	}

	UE_LOG(LogTemp, Display, TEXT("CurrentHp : %f"), CurrentHp);
	if (IsDead() == false)
	{
		if (CurrentHp <= 0)
		{
			Dead(DamageCauser);
		}

		if (IsDead() == false)
		{
			Multi_PlaySoundBase(EMonsterSoundType::Dead);
		}
		else
		{
			Multi_PlaySoundBase(EMonsterSoundType::Hit);
		}
	}

	if (CurrentHp >= MaxHp)
	{
		SetCurrentHp(MaxHp);
	}

	return realDamageAmount;
}

int32 AMonster::GetId()
{
	switch (MonsterType)
	{
	case EMonsterType::Clot:
		return 1;
	case EMonsterType::Crawler:
		return 2;
	case EMonsterType::Stalker:
		return 3;
	case EMonsterType::Gorefast:
		return 4;
	case EMonsterType::Bloat:
		return 5;
	case EMonsterType::Siren:
		return 6;
	case EMonsterType::Husk:
		return 7;
	case EMonsterType::Scrake:
		return 8;
	case EMonsterType::Fleshpound:
		return 9;
	case EMonsterType::None:
	default:
		return 0; // 기본값 처리
	}
}

void AMonster::Multicast_HideHeadBoneAndExplode_Implementation(FVector HeadLocation, FName BoneToHide,
                                                               AActor* DamageCauser)
{
	// 모든 클라이언트 (및 서버)에서 실행될 로직
	if (GetMesh()) // 유효성 검사
	{
		GetMesh()->HideBoneByName(BoneToHide, PBO_None);
	}

	// 머리 폭발 사운드도 여기서 재생 (모든 클라이언트가 동일하게 듣도록)
	GetGameInstance()->GetSubsystem<USoundManager>()->Multi_Play3DSound(
		GetGameInstance()->GetSubsystem<UResourceManager>()->LoadSound(
			"/Game/KF/Sound/KF_EnemyGlobalSnd/Sound/Blood_Burst_Cue.Blood_Burst_Cue"), HeadLocation);

	HeadOff(DamageCauser);
}

void AMonster::Dead(AActor* DamageCauser)
{
	if (HasAuthority() == false)
	{
		return;
	}
	if (GetIsAlive() == false)
	{
		return;
	}

	AGameStateBase* GameStateBase = GetWorld()->GetGameState();
	if (GameStateBase)
	{
		// 2. 우리 프로젝트의 커스텀 GameState로 캐스팅합니다.
		AKillingFloorLikeGameState* MyGameState = Cast<AKillingFloorLikeGameState>(GameStateBase);
		if (MyGameState)
		{
			// 3. GameState를 통해 UnitManager에 접근합니다.
			AUnitManager* UnitManager = MyGameState->GetUnitManager(); // GetUnitManager()는 GameState에 미리 만들어 둔 함수
			if (UnitManager)
			{
				// 4. UnitManager에게 사망 사실을 보고합니다.
				UnitManager->HandleMonsterDeath();
			}
		}
	}

	Super::Dead(DamageCauser);
}

void AMonster::Multi_DeadCallback(AActor* DamageCauser)
{
	Super::Multi_DeadCallback(DamageCauser);

	if (AIController)
	{
		AIController->GetBrainComponent()->StopLogic(TEXT("Stop Blackboard"));
	}

	AKillingFloorLikeCharacter* Attacker = Cast<AKillingFloorLikeCharacter>(DamageCauser->GetOwner());
	if (Attacker && HasAuthority())
	{
		Attacker->SetMoney(Attacker->GetMoney() + MonsterData.bounty);
	}
	if (HasAuthority())
	{
		GetGameInstance()->GetSubsystem<UObjectPoolManager>()->ReturnToPoolAfterDelay(this, &TimerHandle, 5);
	}
}

void AMonster::Attack()
{
	if (IsControllable() == false)
	{
		return;
	}

	// 월드에서 현재 위치를 기준으로 검색할 위치와 반경 정의
	FVector SearchOrigin = GetActorLocation(); // 검색 중심점
	float SearchRadius = MonsterData.range; // 검색 반경

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

UAnimMontage* AMonster::GetAnimationMontage(EMonsterAnimationType animType, int32 index)
{
	return AnimationMap[animType].Montages[index];
}

int32 AMonster::GetAnimationMaxIndex(EMonsterAnimationType animType)
{
	return AnimationMap[animType].Montages.Num();
}

UAnimMontage* AMonster::GetHitAnimMontage(AActor* DamageCauser)
{
	FVector NormVec1 = GetActorForwardVector().GetSafeNormal2D();
	FVector NormVec2 = (DamageCauser->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();

	float Dot = FVector::DotProduct(NormVec1, NormVec2);
	float Cross = NormVec1.X * NormVec2.Y - NormVec1.Y * NormVec2.X; // 2D에서의 외적
	float AngleRad = atan2f(Cross, Dot);

	float degree = FMath::RadiansToDegrees(AngleRad);

	UAnimMontage* Montage;

	if (AnimationMap[EMonsterAnimationType::Hit].Montages.Num() <= 1)
	{
		return AnimationMap[EMonsterAnimationType::Hit].Montages[0];
	}

	if (degree < 45 && degree > -45) //앞
	{
		Montage = AnimationMap[EMonsterAnimationType::Hit].Montages[0];
	}
	else if (degree <= -45 && degree > -135) //좌
	{
		Montage = AnimationMap[EMonsterAnimationType::Hit].Montages[1];
	}
	else if (degree <= -135 || degree > 135) //후
	{
		Montage = AnimationMap[EMonsterAnimationType::Hit].Montages[2];
	}
	else //우
	{
		Montage = AnimationMap[EMonsterAnimationType::Hit].Montages[3];
	}

	UE_LOG(LogTemp, Display, TEXT("degree : %f"), degree);

	return Montage;
}

EMonsterType AMonster::GetMonsterType()
{
	return MonsterType;
}

float AMonster::GetBleedOutTime()
{
	return MonsterData.bleed_out_time;
}

float AMonster::GetStunThreshold()
{
	return MonsterData.stun_threshold;
}

void AMonster::SetMonsterType(EMonsterType NewMonsterType)
{
	MonsterType = NewMonsterType;
}

void AMonster::SetMonsterData(const FMonsterData& NewMonsterData)
{
	MonsterData = NewMonsterData;
}

void AMonster::SetBBIsMoveable(bool IsMoveable)
{
	if (AIController && AIController->GetBlackboardComponent())
	{
		AIController->GetBlackboardComponent()->SetValueAsBool("IsMoveable", IsMoveable);
	}
}
