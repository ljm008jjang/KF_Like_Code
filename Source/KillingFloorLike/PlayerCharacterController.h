// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PlayerCharacterController.generated.h"

struct FGameplayMessage_EndMatch;
struct FGameplayTag;
struct FGameplayMessage_GameMode;
class AKillingFloorLikeCharacter;
class AKillingFloorHud;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API APlayerCharacterController : public APlayerController
{
	GENERATED_BODY()

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY(BlueprintGetter = GetKfCharacter)
	AKillingFloorLikeCharacter* KFCharacter;
	
	UPROPERTY()
	AKillingFloorHud* KillingFloorHud;

	UPROPERTY()
	FTimerHandle ExitUITimeHandle;

public:
	UFUNCTION(BlueprintGetter, BlueprintPure)
	AKillingFloorLikeCharacter* GetKfCharacter() const
	{
		return KFCharacter;
	}

	AKillingFloorHud* GetKillingFloorHud() const
	{
		return KillingFloorHud;
	}
	UFUNCTION(Client, Reliable)
	void ShowShopWidget(bool IsVisible);

	void ShowExitUI(FGameplayTag Channel, const FGameplayMessage_EndMatch& Message);
};
