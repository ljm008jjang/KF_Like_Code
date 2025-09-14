// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ShopPerkImage.generated.h"

enum class EPerkType : uint8;
class UImage;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UShopPerkImageWidget : public UUserWidget
{
	GENERATED_BODY()

private:
	UPROPERTY(meta=(BindWidget))
	UImage* PerkIconImage;

public:
	void RefreshShopPerkImage(EPerkType PerkType);
	
};
