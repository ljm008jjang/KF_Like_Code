// Fill out your copyright notice in the Description page of Project Settings.


#include "MoneyObject.h"

#include "KillingFloorLikeCharacter.h"
#include "ObjectPoolManager.h"
#include "PoolableComponent.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "TP_PickUpComponent.h"

class UResourceManager;
class USoundManager;
// Sets default values
AMoneyObject::AMoneyObject()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MainMesh"));
	SetRootComponent(StaticMeshComponent);

	PoolableComponent = CreateDefaultSubobject<UPoolableComponent>(TEXT("PoolableComponent"));
}


// Called when the game starts or when spawned
void AMoneyObject::BeginPlay()
{
	Super::BeginPlay();
	PickUpComponent = GetComponentByClass<UTP_PickUpComponent>();
	PoolableComponent->OnActivated.AddDynamic(this, &AMoneyObject::OnPooledActivate);
	PoolableComponent->OnDeactivated.AddDynamic(this, &AMoneyObject::OnPooledReset);

	SetReplicateMovement(true);
}


void AMoneyObject::OnPooledActivate()
{
	if (PickUpComponent)
	{
		PickUpComponent->OnPickUp.AddDynamic(this, &AMoneyObject::PickUpMoney);
	}
}

void AMoneyObject::OnPooledReset()
{
	if (PickUpComponent)
	{
		PickUpComponent->OnPickUp.RemoveDynamic(this, &AMoneyObject::PickUpMoney);
	}
}

// Called every frame
void AMoneyObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AMoneyObject::PickUpMoney(AKillingFloorLikeCharacter* TargetCharacter)
{
	if (TargetCharacter == nullptr)
	{
		return;
	}

	TargetCharacter->SetMoney(TargetCharacter->GetMoney() + MoneyAmount);

	GetGameInstance()->GetSubsystem<USoundManager>()->Multi_Play3DSound(
		GetGameInstance()->GetSubsystem<UResourceManager>()->LoadSound(
			"/Game/KF/Sound/Inventory/Cash_Pickup_Cue.Cash_Pickup_Cue"), GetActorLocation());

	GetGameInstance()->GetSubsystem<UObjectPoolManager>()->ReturnToPool(this);
}

void AMoneyObject::SetMoneyAmount(int32 Amount)
{
	MoneyAmount = Amount;
}
