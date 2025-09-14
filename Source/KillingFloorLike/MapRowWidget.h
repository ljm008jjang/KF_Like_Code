// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MapRowWidget.generated.h"

class UKFCheckBox;
class UTextBlock;
class UButton;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UMapRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* MapNameText;


	UPROPERTY(BlueprintReadOnly)
	bool Selected = false;
	

	void Setup(class UUserWidget* NewParent, uint32 NewIndex);

	UPROPERTY(meta=(BindWidget))
	UKFCheckBox* RowCheckBox;
	void UpdateVisuals();

private:

	UPROPERTY()
	UUserWidget* Parent;
	uint32 Index;
	
	UFUNCTION()
	void OnStateChanged(bool IsCheck);
};
