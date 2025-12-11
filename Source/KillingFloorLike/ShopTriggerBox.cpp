// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopTriggerBox.h"

#include "CrossHair.h"
#include "KFLikeGameInstance.h"
#include "KillingFloorHud.h"
#include "KillingFloorLikeGameState.h"
#include "PlayerCharacterController.h"
#include "Kismet/GameplayStatics.h"

void AShopTriggerBox::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	if (OtherActor->HasAuthority() == false)
	{
		return;
	}


	if (Cast<AKillingFloorLikeGameState>(UGameplayStatics::GetGameState(GetWorld()))->GetCurrentModeType() ==
		EModeType::Break == false)
	{
		return;
	}

	AKillingFloorLikeCharacter* KFCharacter = Cast<AKillingFloorLikeCharacter>(OtherActor);
	if (IsValid(KFCharacter) == false)
	{
		return;
	}

	KFCharacter->SetIsShopable(true);
	KFCharacter->GetPlayerCharacterController()->ShowShopWidget(true);
}

void AShopTriggerBox::Client_ActorOverlapCallback_Implementation(AKillingFloorLikeCharacter* OtherActor,
                                                                 bool bIsOverlapped)
{
	if (OtherActor->IsLocallyControlled())
	{
		OtherActor->GetPlayerCharacterController()->GetKillingFloorHud()->GetWbCrossHairInstance()->
		            UpdateTradeTextVisibility(bIsOverlapped);
	}
}

void AShopTriggerBox::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);
	if (OtherActor->HasAuthority() == false)
	{
		return;
	}

	AKillingFloorLikeCharacter* KFCharacter = Cast<AKillingFloorLikeCharacter>(OtherActor);
	if (IsValid(KFCharacter) == false)
	{
		return;
	}

	KFCharacter->SetIsShopable(false);
	//Client_ActorOverlapCallback(KFCharacter, false);
	KFCharacter->GetPlayerCharacterController()->ShowShopWidget(false);
}
