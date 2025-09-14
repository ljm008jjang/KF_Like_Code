// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseWeapon.h"
#include "Blueprint/UserWidget.h"
#include "ShopPurchaseWeaponWidget.generated.h"

class UCheckBox;
class UShopWidget;
class UTextBlock;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UShopPurchaseWeaponWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY()
	FWeaponData WeaponData;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* NameText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* CostText;

	UPROPERTY(BlueprintGetter=GetShopWidget, BlueprintSetter=SetShopWidget)
	UShopWidget* ShopWidget;

public:
	UFUNCTION(BlueprintCallable)
	void Init(const FWeaponData& NewWeaponData, UShopWidget* NewShopWidget);
	
	void Refresh();

	UFUNCTION(BlueprintGetter)
	UShopWidget* GetShopWidget() const
	{
		return ShopWidget;
	}

	UFUNCTION(BlueprintSetter)
	void SetShopWidget(UShopWidget* NewShopWidget)
	{
		this->ShopWidget = NewShopWidget;
	}
	
	UPROPERTY(meta=(BindWidget))
	UCheckBox* WeaponCheckBox;

	FWeaponData GetWeaponData() const
	{
		return WeaponData;
	}
};
