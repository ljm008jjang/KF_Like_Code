// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "PlayerAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UPlayerAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

private:

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	UBlendSpace* CurrentLocomotionBS;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintGetter=GetSpeed)
	float Speed;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintGetter=GetAngle)
	float Angle;

	UFUNCTION(BlueprintGetter)
	float GetSpeed()
	{
		return Speed;
	}

	UFUNCTION(BlueprintGetter)
	float GetAngle()
	{
		return Angle;
	}
};
