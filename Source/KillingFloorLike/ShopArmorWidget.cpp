// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopArmorWidget.h"

#include "BasicButtonWidget.h"
#include "PubSubManager.h"
#include "ShopWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

class UPubSubManager;

void UShopArmorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Subscribe(EPubSubTag::Player_HealthChange, this,
	                                                             &UShopArmorWidget::UpdateArmorText);
	OneButton->Button->OnClicked.AddDynamic(this, &UShopArmorWidget::BuyArmor);

	OneButton->SetText(FText::AsNumber(300));
}

void UShopArmorWidget::NativeDestruct()
{
	OneButton->Button->OnClicked.RemoveDynamic(this, &UShopArmorWidget::BuyArmor);
	Super::NativeDestruct();
}

bool UShopArmorWidget::Initialize()
{
	bool Result = Super::Initialize();

	return Result;
}

void UShopArmorWidget::Refresh(UShopWidget* NewShopWidget)
{
	if (IsValid(NewShopWidget))
	{
		ShopWidget = NewShopWidget;
	}

	WeaponNameText->SetText(FText::FromString("Armor"));
	
	UpdateArmorText(Cast<AKillingFloorLikeCharacter>(GetOwningPlayerPawn())->GetCurrentArmor());
}

void UShopArmorWidget::BuyArmor()
{
	Cast<AKillingFloorLikeCharacter>(GetOwningPlayerPawn())->Server_BuyArmor();
}

void UShopArmorWidget::UpdateArmorText(FGameplayTag Channel, const FGameplayMessage_PlayerHeathChange& Message)
{
	if (Message.ListenerObject != GetOwningPlayerPawn())
	{
		return;
	}

	UpdateArmorText(Message.Armor);
}

void UShopArmorWidget::UpdateArmorText(float Armor)
{
	AmmoText->SetText(FText::AsNumber(Armor));
}