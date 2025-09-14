// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "BasicButtonWidget.generated.h"

class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UBasicButtonWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	UButton* Button;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* Text;

	UFUNCTION(BlueprintCallable)
	void SetText(FText NewText)
	{
		Text->SetText(NewText);
	}
};
