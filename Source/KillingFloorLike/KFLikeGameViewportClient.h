// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "KFLikeGameViewportClient.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UKFLikeGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
public:
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
};
