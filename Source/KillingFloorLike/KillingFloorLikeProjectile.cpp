// Copyright Epic Games, Inc. All Rights Reserved.

#include "KillingFloorLikeProjectile.h"

#include "Monster.h"
#include "ObjectPoolManager.h"
#include "PoolableComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet/GameplayStatics.h"

AKillingFloorLikeProjectile::AKillingFloorLikeProjectile()
{
	// Use a sphere as a simple collision representation
	CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");	
	// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 3000.f;
	ProjectileMovement->MaxSpeed = 3000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false; // 벽에 닿았을 때 튕겨나가지 않고 멈추도록 설정

	PoolableComponent = CreateDefaultSubobject<UPoolableComponent>(TEXT("PoolableComponent"));

	bReplicates = true;
	ProjectileMovement->SetIsReplicated(true);

	// Die after 3 seconds by default
	// InitialLifeSpan = 3.0f;
}


void AKillingFloorLikeProjectile::BeginPlay()
{
	Super::BeginPlay();

	SetReplicateMovement(true);

	PoolableComponent->OnActivated.AddDynamic(this, &AKillingFloorLikeProjectile::OnPooledActivate);
	PoolableComponent->OnDeactivated.AddDynamic(this, &AKillingFloorLikeProjectile::OnPooledReset);

	// OnHit 이벤트를 바인딩합니다. 서버에서만 충돌 로직을 처리하도록 합니다.
	if (HasAuthority())
	{
		GetCollisionComp()->OnComponentBeginOverlap.AddDynamic(this, &AKillingFloorLikeProjectile::OnOverlapBegin);
		GetCollisionComp()->OnComponentHit.AddDynamic(this, &AKillingFloorLikeProjectile::OnHit);
	}
}

void AKillingFloorLikeProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	Super::EndPlay(EndPlayReason);
}


void AKillingFloorLikeProjectile::OnPooledActivate()
{
	Cast<UPrimitiveComponent>(GetRootComponent())->SetSimulatePhysics(false);
	ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
	ProjectileMovement->SetUpdatedComponent(CollisionComp);
	ProjectileMovement->Activate(); // ← 반드시 호출!
	//OnPooledActivateCallback();

	GetGameInstance()->GetSubsystem<UObjectPoolManager>()->ReturnToPoolAfterDelay(this, &TimerHandle, 5);
}

void AKillingFloorLikeProjectile::OnPooledActivateCallback_Implementation()
{
	Cast<UPrimitiveComponent>(GetRootComponent())->SetSimulatePhysics(false);
	ProjectileMovement->Velocity = GetActorForwardVector() * ProjectileMovement->InitialSpeed;
	ProjectileMovement->SetUpdatedComponent(CollisionComp);
	ProjectileMovement->Activate(); // ← 반드시 호출!
}

void AKillingFloorLikeProjectile::OnPooledReset()
{
	HittedActor.Empty();
	CurrentPenetrationCount = 0;
	ProjectileMovement->Deactivate(); // ← 반드시 호출!
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void AKillingFloorLikeProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// 서버에서만 충돌 로직을 실행합니다.
	if (!HasAuthority())
	{
		return;
	}

	// 캐릭터를 맞춘 경우는 OnOverlapBegin에서 이미 처리하고 있으므로,
	// 여기서는 캐릭터가 아닌 다른 오브젝트(벽, 바닥 등)에 부딪혔을 때만 처리합니다.
	// 이렇게 하면 캐릭터 관통 로직과 충돌하지 않습니다.
	if (IsValid(Cast<ABaseCharacter>(OtherActor)) == false)
	{
		// 부딪힌 대상이 캐릭터가 아니면, 즉시 풀로 돌아갑니다.
		UE_LOG(LogTemp, Log, TEXT("Projectile %s blocked by %s. Returning to pool."), *GetName(), *GetNameSafe(OtherActor));
		GetGameInstance()->GetSubsystem<UObjectPoolManager>()->ReturnToPool(this);
	}
	// 만약 부딪힌 대상이 캐릭터라면, OnOverlapBegin에서 처리될 것이므로 여기서는 아무것도 하지 않습니다.
	// 투사체는 캐릭터를 관통하거나, 관통 횟수를 다 소모하면 OnOverlapBegin 내부에서 풀로 돌아가게 됩니다.
}

void AKillingFloorLikeProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                                 UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                                 const FHitResult& SweepResult)
{
	if (HasAuthority() == false)
	{
		return;
	}
	
	ABaseCharacter* ShooterInstigator = Cast<ABaseCharacter>(GetInstigator());
	ABaseCharacter* OtherCharacter = Cast<ABaseCharacter>(OtherActor);
	if (IsValid(ShooterInstigator) == false || IsValid(OtherCharacter) == false)
	{
		return;
	}

	if (ShooterInstigator->IsAttackableUnitType(OtherCharacter) == false)
	{
		return;
	}

	if (IsHittedActor(OtherCharacter))
	{
		return;
	}

	FVector HitFromDirection = GetActorLocation() - OtherActor->GetActorLocation();
	UGameplayStatics::ApplyPointDamage(OtherCharacter, GetProjectileDamage(), HitFromDirection, SweepResult,
	                                   GetInstigatorController(), GetOwner(), UDamageType::StaticClass());
	AddHittedActor(OtherCharacter);
	AddCurrentPenetrationCount(1);
	if (IsPenetrateOver())
	{
		GetGameInstance()->GetSubsystem<UObjectPoolManager>()->ReturnToPool(this);
	}
}

void AKillingFloorLikeProjectile::Initialize(AActor* NewOwner, APawn* NewInstigator, float GunDamage,
                                             int NewPenteration, float NewPenterationDamage)
{
	if (NewOwner)
	{
		SetOwner(NewOwner);
	}
	if (NewInstigator)
	{
		SetInstigator(NewInstigator);
	}
	ProjectileDamage = GunDamage;
	Penetration = NewPenteration;
	PenetrationDamageRatio = NewPenterationDamage;
}

float AKillingFloorLikeProjectile::GetProjectileDamage()
{
	return ProjectileDamage * pow(PenetrationDamageRatio, CurrentPenetrationCount);
}

int32 AKillingFloorLikeProjectile::GetCurrentPenetrationCount()
{
	return CurrentPenetrationCount;
}

void AKillingFloorLikeProjectile::AddCurrentPenetrationCount(int32 AddPenetrationCount)
{
	CurrentPenetrationCount += AddPenetrationCount;
}

bool AKillingFloorLikeProjectile::IsPenetrateOver()
{
	return CurrentPenetrationCount >= Penetration;
}

void AKillingFloorLikeProjectile::AddHittedActor(AActor* AddActor)
{
	if (HittedActor.Contains(AddActor))
	{
		return;
	}

	HittedActor.Add(AddActor);
}

bool AKillingFloorLikeProjectile::IsHittedActor(AActor* HitActor)
{
	if (HittedActor.Contains(HitActor))
	{
		return true;
	}
	return false;
}
