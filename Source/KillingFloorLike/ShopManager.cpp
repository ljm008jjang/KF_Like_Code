// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopManager.h"

#include "SplineActor.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

// Sets default values
AShopManager::AShopManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	bReplicates = true;
}

void AShopManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// CurrentTimeDilation 변수를 모든 클라이언트에게 복제합니다.
	DOREPLIFETIME(AShopManager, CurrentShop);
}

// Called when the game starts or when spawned
void AShopManager::BeginPlay()
{
	Super::BeginPlay();

	SplineActor = Cast<ASplineActor>(UGameplayStatics::GetActorOfClass(GetWorld(), ASplineActor::StaticClass()));
}

void AShopManager::SetNewShop()
{
	CurrentShop = ShopComponents[FMath::RandHelper(ShopComponents.Max() - 1)];
}

AActor* AShopManager::GetCurrentShop() const
{
	return CurrentShop;
}

FText AShopManager::GetShopDistText()
{
	if (IsValid(Character) == false)
	{
		Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	}
	if (IsValid(CurrentShop) == false || IsValid(Character) == false)
	{
		return FText::FromString("Trader: ??m");
	}

	float dist = FVector::Dist(Character->GetActorLocation(), CurrentShop->GetActorLocation());

	int32 distMeter = (int32)(dist * 0.01f);

	return FText::FromString("Trader: " + FString::FromInt(distMeter) + "m");
}
