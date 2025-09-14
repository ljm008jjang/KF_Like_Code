// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopArmorWidget.generated.h"

struct FGameplayMessage_PlayerHeathChange;
struct FGameplayMessage_None;
struct FGameplayMessage_PlayerMoneyChange;
struct FGameplayTag;
class UShopWidget;
class UCheckBox;
class UBasicButtonWidget;
class UTextBlock;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UShopArmorWidget : public UUserWidget
{
	GENERATED_BODY()
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual bool Initialize() override;

private:
	UPROPERTY(BlueprintGetter=GetShopWidget) //BlueprintGetter=GetShopWidget, BlueprintSetter=SetShopWidget)
	UShopWidget* ShopWidget;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeaponNameText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* AmmoText;
	
	UPROPERTY(meta=(BindWidget))
	UBasicButtonWidget* OneButton;

	UFUNCTION()
	void BuyArmor();

public:
	UPROPERTY(meta=(BindWidget))
	UCheckBox* WeaponCheckBox;

	UFUNCTION(BlueprintGetter, BlueprintPure)
	UShopWidget* GetShopWidget() const
	{
		return ShopWidget;
	}

	UFUNCTION(blueprintCallable)
	void Refresh(UShopWidget* NewShopWidget = nullptr);
	
	void UpdateArmorText(FGameplayTag Channel, const FGameplayMessage_PlayerHeathChange& Message);
	void UpdateArmorText(float Armor);
};
