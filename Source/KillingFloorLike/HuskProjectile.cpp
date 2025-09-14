// Fill out your copyright notice in the Description page of Project Settings.


#include "HuskProjectile.h"

#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "BaseCharacter.h"
#include "CollisionQueryParams.h"
#include "FireDamageType.h"
#include "ObjectPoolManager.h"
#include "Engine/OverlapResult.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"


AHuskProjectile::AHuskProjectile()
{
	// Use a sphere as a simple collision representation
	//CollisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	CollisionComp->InitSphereRadius(5.0f);
	CollisionComp->BodyInstance.SetCollisionProfileName("Projectile");
	//CollisionComp->OnComponentHit.AddDynamic(this, &AKillingFloorLikeProjectile::OnHit);
	// set up a notification for when this component hits something blocking

	// Players can't walk on it
	CollisionComp->SetWalkableSlopeOverride(FWalkableSlopeOverride(WalkableSlope_Unwalkable, 0.f));
	CollisionComp->CanCharacterStepUpOn = ECB_No;

	// Set as root component
	RootComponent = CollisionComp;

	// Use a ProjectileMovementComponent to govern this projectile's movement
	//ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileComp"));
	ProjectileMovement->UpdatedComponent = CollisionComp;
	ProjectileMovement->InitialSpeed = 2000.f;
	ProjectileMovement->MaxSpeed = 2000.f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = true;

	// Die after 3 seconds by default
	// InitialLifeSpan = 3.0f;
}


void AHuskProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		GetCollisionComp()->OnComponentHit.AddDynamic(this, &AHuskProjectile::OnHit);
	}
	NiagaraComp = GetComponentByClass<UNiagaraComponent>();
}

void AHuskProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	GetCollisionComp()->OnComponentHit.RemoveDynamic(this, &AHuskProjectile::OnHit);
	Super::EndPlay(EndPlayReason);
}

void AHuskProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	const ABaseCharacter* ShooterInstigator = Cast<ABaseCharacter>(GetInstigator());
	if (ShooterInstigator == nullptr)
	{
		return;
	}

	if (ShooterInstigator == OtherActor)
	{
		return;
	}

	// 2. 물리 반동 주기
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(DamageRadius);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn, // 또는 ECC_PhysicsBody
		Sphere
	);

	if (bHit)
	{
		for (const FOverlapResult& Result : Overlaps)
		{
			AActor* HitActor = Result.GetActor();
			if (HitActor && HitActor != this)
			{
				// 조건 추가 가능 (예: 특정 클래스, 팀 등)
				UGameplayStatics::ApplyDamage(
					HitActor,
					GetProjectileDamage(),
					GetInstigatorController(),
					GetOwner(),
					UFireDamageType::StaticClass()
				);

				// 밀어내기 효과
				UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(HitActor->GetRootComponent());
				if (RootComp && RootComp->IsSimulatingPhysics())
				{
					FVector Direction = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
					RootComp->AddImpulse(Direction * KnockbackStrength, NAME_None, true);
				}
			}
		}
		ProjectileMovement->Deactivate(); // ← 반드시 호출!
		//NiagaraComp->
		GetGameInstance()->GetSubsystem<UObjectPoolManager>()->ReturnToPoolAfterDelay(this, &TimerHandle, 1);
	}
}

void AHuskProjectile::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                     const FHitResult& SweepResult)
{
	const ABaseCharacter* ShooterInstigator = Cast<ABaseCharacter>(GetInstigator());
	if (ShooterInstigator == nullptr)
	{
		return;
	}

	if (ShooterInstigator == OtherActor)
	{
		return;
	}

	// 2. 물리 반동 주기
	TArray<FOverlapResult> Overlaps;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(DamageRadius);

	bool bHit = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		GetActorLocation(),
		FQuat::Identity,
		ECC_Pawn, // 또는 ECC_PhysicsBody
		Sphere
	);

	if (bHit)
	{
		for (const FOverlapResult& Result : Overlaps)
		{
			AActor* HitActor = Result.GetActor();
			if (HitActor && HitActor != this)
			{
				// 조건 추가 가능 (예: 특정 클래스, 팀 등)
				UGameplayStatics::ApplyDamage(
					HitActor,
					GetProjectileDamage(),
					GetInstigatorController(),
					GetOwner(),
					UFireDamageType::StaticClass()
				);

				// 밀어내기 효과
				UPrimitiveComponent* RootComp = Cast<UPrimitiveComponent>(HitActor->GetRootComponent());
				if (RootComp && RootComp->IsSimulatingPhysics())
				{
					FVector Direction = (HitActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
					RootComp->AddImpulse(Direction * KnockbackStrength, NAME_None, true);
				}
			}
		}
		GetGameInstance()->GetSubsystem<UObjectPoolManager>()->ReturnToPool(this);
	}
}
