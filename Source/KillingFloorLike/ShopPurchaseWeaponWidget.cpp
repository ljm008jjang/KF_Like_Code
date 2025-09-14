// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopPurchaseWeaponWidget.h"

#include "KillingFloorLikeCharacter.h"
#include "PlayerCharacterController.h"
#include "ShopWidget.h"
#include "Components/TextBlock.h"

void UShopPurchaseWeaponWidget::Init(const FWeaponData& NewWeaponData, UShopWidget* NewShopWidget)
{
	WeaponData = NewWeaponData;
	ShopWidget = NewShopWidget;
	Refresh();
}

void UShopPurchaseWeaponWidget::Refresh()
{
	
	NameText->SetText(FText::FromString(WeaponData.name));

	CostText->SetText(
		FText::AsNumber(ShopWidget->GetPlayerCharacterController()->GetKfCharacter()->GetWeaponCost(WeaponData)));
}
