// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopQuickPerkSelectWidget.h"

#include "KFLikeGameInstance.h"
#include "ResourceManager.h"
#include "Components/Button.h"
#include "Components/Image.h"

void UShopQuickPerkSelectWidget::NativeConstruct()
{
	Super::NativeConstruct();
	PerkButton->OnClicked.AddDynamic(this, &UShopQuickPerkSelectWidget::OnButtonClicked);
}

void UShopQuickPerkSelectWidget::NativeDestruct()
{
	Super::NativeDestruct();
	PerkButton->OnClicked.RemoveDynamic(this, &UShopQuickPerkSelectWidget::OnButtonClicked);
}

void UShopQuickPerkSelectWidget::Init(EPerkType NewPerkType, AKillingFloorLikeCharacter* NewKFCharacter)
{
	KFCharacter = NewKFCharacter;
	PerkType = NewPerkType;

	UResourceManager* ResourceManager = GetGameInstance()->GetSubsystem<UResourceManager>();
	
	if (IsValid(ResourceManager) == false)
	{
		return;
	}

	PerkImage->SetBrushFromTexture(ResourceManager->LoadPerkIconTexture(PerkType));
}

void UShopQuickPerkSelectWidget::OnButtonClicked()
{
	if (IsValid(KFCharacter) == false)
	{
		return;
	}

	KFCharacter->Server_ChangePerk(PerkType, 6);
}
