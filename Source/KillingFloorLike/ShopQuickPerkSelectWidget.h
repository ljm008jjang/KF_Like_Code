// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopQuickPerkSelectWidget.generated.h"

class AKillingFloorLikeCharacter;
class UButton;
class UImage;
enum class EPerkType : uint8;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UShopQuickPerkSelectWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY()
	AKillingFloorLikeCharacter* KFCharacter;

	UPROPERTY()
	EPerkType PerkType;

	UPROPERTY(meta=(BindWidget))
	UImage* PerkImage;

	UPROPERTY(meta=(BindWidget))
	UButton* PerkButton;

public:
	void Init(EPerkType NewPerkType, AKillingFloorLikeCharacter* NewKFCharacter);
	UFUNCTION()
	void OnButtonClicked();

	EPerkType GetPerkType() const
	{
		return PerkType;
	}
};
