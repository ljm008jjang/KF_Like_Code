// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseGameConditionInterface.h"
#include "Components/SceneComponent.h"
#include "GameWaveComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class KILLINGFLOORLIKE_API UGameWaveComponent : public USceneComponent, public IBaseGameConditionInterface
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UGameWaveComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
