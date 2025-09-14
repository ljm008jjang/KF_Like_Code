// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KillingFloorLikeCharacter.h"
#include "PubSubManager.h"
#include "Blueprint/UserWidget.h"
#include "PerkIconWithStarWidget.generated.h"

class UImage;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UPerkIconWithStarWidget : public UUserWidget
{
	GENERATED_BODY()
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta=(BindWidget))
	UImage* PerkImage;

	UPROPERTY()
	TArray<UImage*> StarImageArray;

public:
	void SetPerk(FGameplayTag Channel, const FGameplayMessage_Perk& Message);
	void SetPerk(const FPerkData& PerkData);
};
