// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "ShopTriggerBox.generated.h"

class AKillingFloorLikeCharacter;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API AShopTriggerBox : public ATriggerBox
{
	GENERATED_BODY()

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;

	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;

	UFUNCTION(Client, Reliable)
	void Client_ActorOverlapCallback(AKillingFloorLikeCharacter* OtherActor, bool bIsOverlapped);
	
};
