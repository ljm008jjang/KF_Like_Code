// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/CheckBox.h"
#include "KFCheckBox.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UKFCheckBox : public UCheckBox
{
	GENERATED_BODY()
	UKFCheckBox(const FObjectInitializer& ObjectInitializer);
};
