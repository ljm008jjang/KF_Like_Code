// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_BlueprintBase.h"
#include "BTTaskNodeAttack.generated.h"

class AMonster;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UBTTaskNodeAttack : public UBTTask_BlueprintBase
{
	GENERATED_BODY()

	UFUNCTION(blueprintCallable)
	void PlayAttackMontage(AMonster* MeshComp, UAnimMontage* Montage);
};
