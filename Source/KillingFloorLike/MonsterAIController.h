// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MonsterAIController.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API AMonsterAIController : public AAIController
{
	GENERATED_BODY()

	virtual void Tick(float DeltaSeconds) override;
	//virtual void OnPossess(APawn* InPawn) override;

private:
	UPROPERTY(BlueprintGetter = GetIsRotate, BlueprintSetter = SetIsRotate)
	bool IsRotate = false;

	void SetActorRotation(float DeltaSeconds);

public:
	UFUNCTION(BlueprintGetter)
	bool GetIsRotate() const
	{
		return IsRotate;
	}

	UFUNCTION(BlueprintSetter)
	void SetIsRotate(bool bIsRotate)
	{
		IsRotate = bIsRotate;
	}

	void OnDead();
};
