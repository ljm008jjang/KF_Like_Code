// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopPlayerWeaponWidget.h"

#include "BaseWeapon.h"
#include "KillingFloorLikeCharacter.h"
#include "PubSubManager.h"
#include "ConsumableWeapon.h"
#include "BasicButtonWidget.h"
#include "ShopWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UShopPlayerWeaponWidget::NativeConstruct()
{
	Super::NativeConstruct();

	//UE_LOG(LogTemp, Warning, TEXT("%s"), *GetOuter()->GetName());
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Subscribe(EPubSubTag::Player_AmmoChange, this,
	                                                             &UShopPlayerWeaponWidget::UpdateAmmoText);
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Subscribe(EPubSubTag::Player_MoneyChange, this,
	                                                             &UShopPlayerWeaponWidget::UpdateMoneyText);

	MagButton->Button->OnClicked.AddDynamic(this, &UShopPlayerWeaponWidget::BuyAmmoMag);
	FillButton->Button->OnClicked.AddDynamic(this, &UShopPlayerWeaponWidget::BuyAmmoFill);
}

void UShopPlayerWeaponWidget::NativeDestruct()
{
	/*if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPubSubManager* PubSubManager = GameInstance->GetSubsystem<UPubSubManager>())
		{
			// 이 위젯(this)에 등록된 모든 구독을 해지합니다.
			PubSubManager->Unsubscribe(this);
		}
	}*/
	MagButton->Button->OnClicked.RemoveDynamic(this, &UShopPlayerWeaponWidget::BuyAmmoMag);
	FillButton->Button->OnClicked.RemoveDynamic(this, &UShopPlayerWeaponWidget::BuyAmmoFill);
	Super::NativeDestruct();
}

bool UShopPlayerWeaponWidget::Initialize()
{
	bool Result = Super::Initialize();

	OneButton->SetVisibility(ESlateVisibility::Collapsed);

	return Result;
}

void UShopPlayerWeaponWidget::Refresh(ABaseWeapon* NewWeapon, UShopWidget* NewShopWidget)
{
	if (IsValid(NewShopWidget))
	{
		ShopWidget = NewShopWidget;
	}
	if (IsValid(NewWeapon) == false)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}
	SetWeapon(NewWeapon);

	WeaponNameText->SetText(FText::FromString(NewWeapon->GetWeaponData().name));

	UpdateMoneyText();
	UpdateAmmoText();
	SetMagText();
	SetVisibility(ESlateVisibility::Visible);
}

void UShopPlayerWeaponWidget::UpdateAmmoText(FGameplayTag Channel, const FGameplayMessage_PlayerAmmoChange& Message)
{
	if (Message.ListenerObject != GetOwningPlayerPawn())
	{
		return;
	}

	if (Message.Weapon.Get() != Weapon)
	{
		return;
	}

	UpdateAmmoText();
}

void UShopPlayerWeaponWidget::UpdateAmmoText()
{
	AConsumableWeapon* ConsumableWeapon = Cast<AConsumableWeapon>(Weapon);
	if (IsValid(ConsumableWeapon) == false)
	{
		AmmoText->SetText(FText::GetEmpty());
		return;
	}

	AmmoText->SetText(ConsumableWeapon->GetShopAmmoText());
}

void UShopPlayerWeaponWidget::UpdateMoneyText(FGameplayTag Channel, const FGameplayMessage_PlayerMoneyChange& Message)
{
	if (Message.ListenerObject != GetOwningPlayerPawn())
	{
		return;
	}

	UpdateMoneyText();
}

void UShopPlayerWeaponWidget::UpdateMoneyText()
{
	AConsumableWeapon* ConsumableWeapon = Cast<AConsumableWeapon>(Weapon);
	if (IsValid(ConsumableWeapon) == false)
	{
		FillButton->Text->SetText(FText::GetEmpty());
		return;
	}

	FillButton->Text->SetText(FText::AsNumber(ConsumableWeapon->GetAmmoFillCost()));
}

void UShopPlayerWeaponWidget::SetMagText()
{
	if (IsValid(Weapon) == false)
	{
		return;
	}
	AConsumableWeapon* ConsumableWeapon = Cast<AConsumableWeapon>(Weapon);
	if (IsValid(ConsumableWeapon) == false)
	{
		MagButton->Text->SetText(FText::GetEmpty());
		return;
	}

	MagButton->Text->SetText(FText::AsNumber(ConsumableWeapon->GetAmmoMagCost()));
}

void UShopPlayerWeaponWidget::SetOnButton(bool IsOnButton)
{
	if (IsOnButton)
	{
		OneButton->SetVisibility(ESlateVisibility::Visible);
		MagButton->SetVisibility(ESlateVisibility::Collapsed);
		FillButton->SetVisibility(ESlateVisibility::Collapsed);
	}else
	{
		OneButton->SetVisibility(ESlateVisibility::Collapsed);
		MagButton->SetVisibility(ESlateVisibility::Visible);
		FillButton->SetVisibility(ESlateVisibility::Visible);
	}
}

void UShopPlayerWeaponWidget::SetButtonsNotVisible()
{
	//OneButton->SetVisibility(ESlateVisibility::Collapsed);
	MagButton->SetVisibility(ESlateVisibility::Hidden);
	FillButton->SetVisibility(ESlateVisibility::Hidden);
}

void UShopPlayerWeaponWidget::BuyAmmoMag()
{
	Weapon->GetCharacter()->Server_BuyWeaponMag(Weapon);
}

/*void UShopPlayerWeaponWidget::UpdateAmmoText(FGameplayTag Channel, const FGameplayMessage_None& Message)
{
	if (Message.ListenerObject != GetOwningPlayerPawn())
	{
		return;
	}
	UpdateAmmoText(Channel, Message);
}*/

/*
void UShopPlayerWeaponWidget::UpdateMoneyText(FGameplayTag Channel, const FGameplayMessage_PlayerMoneyChange& Message)
{
	if (Message.ListenerObject != GetOwningPlayerPawn())
	{
		return;
	}
	UpdateMoneyText(Channel, Message);
}
*/


void UShopPlayerWeaponWidget::BuyAmmoFill()
{
	if (AConsumableWeapon* ConsumableWeapon = Cast<AConsumableWeapon>(Weapon))
	{
		ConsumableWeapon->Server_BuyWeaponFill();
	}
}
