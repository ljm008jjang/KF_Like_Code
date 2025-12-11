// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopPerkImage.h"

#include "KFLikeGameInstance.h"
#include "ResourceManager.h"
#include "Components/Image.h"

void UShopPerkImageWidget::RefreshShopPerkImage(EPerkType PerkType)
{
	UResourceManager* ResourceManager = GetGameInstance()->GetSubsystem<UResourceManager>();
	
	if (IsValid(ResourceManager) == false)
	{
		return;
	}


	PerkIconImage->SetBrushFromTexture(ResourceManager->LoadPerkIconTexture(PerkType));
}
