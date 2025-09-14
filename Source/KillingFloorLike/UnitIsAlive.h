// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "UnitIsAlive.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UUnitIsAlive : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UUnitIsAlive(const FObjectInitializer& ObjectInitializer);

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
};
