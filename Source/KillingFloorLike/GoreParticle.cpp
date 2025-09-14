// Fill out your copyright notice in the Description page of Project Settings.


#include "GoreParticle.h"

#include "ObjectPoolManager.h"
#include "PoolableComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AGoreParticle::AGoreParticle()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	PoolableComponent = CreateDefaultSubobject<UPoolableComponent>(TEXT("PoolableComponent"));
}

void AGoreParticle::BeginPlay()
{
	Super::BeginPlay();
	SetReplicateMovement(true);

	PoolableComponent->OnActivated.AddDynamic(this, & AGoreParticle::OnPooledActivate);
}

void AGoreParticle::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	Super::EndPlay(EndPlayReason);
}

void AGoreParticle::OnPooledActivate()
{
	SetCurrentMeshIndex(FMath::RandHelper(BrainParticleStaticMeshes.Num()));
	GetGameInstance()->GetSubsystem<UObjectPoolManager>()->ReturnToPoolAfterDelay(this, &TimerHandle, 5);
}

void AGoreParticle::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// CurrentTimeDilation 변수를 모든 클라이언트에게 복제합니다.
	DOREPLIFETIME(AGoreParticle, CurrentMeshIndex);
}

void AGoreParticle::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	StaticMeshComponent = FindComponentByClass<UStaticMeshComponent>();
}

void AGoreParticle::OnRep_CurrentMeshIndex()
{
	// 클라이언트에서 CurrentMeshIndex가 변경될 때 호출됨
	if (StaticMeshComponent && BrainParticleStaticMeshes.IsValidIndex(CurrentMeshIndex))
	{
		StaticMeshComponent->SetStaticMesh(BrainParticleStaticMeshes[CurrentMeshIndex]);
	}
}

void AGoreParticle::Multi_AddImpurse_Implementation(FVector HeadLocation)
{
	UE_LOG(LogTemp, Warning, TEXT("Client_AddImpurse_Implementation"));
	UStaticMeshComponent* x = GetComponentByClass<UStaticMeshComponent>();
	x->AddRadialImpulse(HeadLocation, 1, 50, RIF_Constant);
}
