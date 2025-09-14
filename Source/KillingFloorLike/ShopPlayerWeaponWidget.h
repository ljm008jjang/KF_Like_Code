// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PubSubManager.h"
#include "Blueprint/UserWidget.h"
#include "ShopPlayerWeaponWidget.generated.h"

class UBasicButtonWidget;
class UButton;
class UShopWidget;
class UCheckBox;
class UTextBlock;
class ABaseWeapon;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UShopPlayerWeaponWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual bool Initialize() override;

private:
	UPROPERTY(BlueprintGetter=GetWeapon, BlueprintSetter=SetWeapon)
	ABaseWeapon* Weapon;

	UPROPERTY(BlueprintGetter=GetShopWidget) //BlueprintGetter=GetShopWidget, BlueprintSetter=SetShopWidget)
	UShopWidget* ShopWidget;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeaponNameText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* AmmoText;

	UPROPERTY(meta=(BindWidget))
	UBasicButtonWidget* MagButton;
	UPROPERTY(meta=(BindWidget))
	UBasicButtonWidget* FillButton;

	UPROPERTY(meta=(BindWidget))
	UBasicButtonWidget* OneButton;

public:
	UPROPERTY(meta=(BindWidget))
	UCheckBox* WeaponCheckBox;

	UFUNCTION(BlueprintGetter, BlueprintPure)
	UShopWidget* GetShopWidget() const
	{
		return ShopWidget;
	}

	UFUNCTION(BlueprintGetter, BlueprintPure)
	ABaseWeapon* GetWeapon() const
	{
		return Weapon;
	}

	UFUNCTION(BlueprintSetter)
	void SetWeapon(ABaseWeapon* NewWeapon)
	{
		this->Weapon = NewWeapon;
	}

	UFUNCTION(blueprintCallable)
	void Refresh(ABaseWeapon* NewWeapon, UShopWidget* NewShopWidget = nullptr);

	UFUNCTION()
	void BuyAmmoFill();
	UFUNCTION()
	void BuyAmmoMag();


	void UpdateMoneyText(FGameplayTag Channel, const FGameplayMessage_PlayerMoneyChange& Message);
	void UpdateAmmoText(FGameplayTag Channel, const FGameplayMessage_PlayerAmmoChange& Message);
	void UpdateAmmoText();
	//void UpdateMoneyText(FGameplayTag Channel, const FGameplayMessage_None& Message);
	void UpdateMoneyText();

	UFUNCTION(BlueprintCallable)
	void SetMagText();

	void SetOnButton(bool IsOnButton);
	void SetButtonsNotVisible();
};
