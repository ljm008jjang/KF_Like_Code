// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "BasicCheckBoxWidget.generated.h"

class UCheckBox;
class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UBasicCheckBoxWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta=(BindWidget))
	UCheckBox* CheckBox;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* Text;

	UFUNCTION(BlueprintCallable)
	void SetText(FText NewText)
	{
		Text->SetText(NewText);
	}
};
