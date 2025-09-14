// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ServerRowWidget.generated.h"

class UJoinServerWidget;
class UMainWidget;
class UButton;
class UTextBlock;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UServerRowWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ServerNameText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* MapNameText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* PlayersText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WaveText;

	UPROPERTY(BlueprintReadOnly)
	bool Selected = false;
	

	void Setup(class UJoinServerWidget* NewParent, uint32 NewIndex);

private:
	UPROPERTY(meta=(BindWidget))
	UButton* RowButton;

	UPROPERTY()
	UJoinServerWidget* Parent;
	uint32 Index;
	
	UFUNCTION()
	void OnClicked();
};
