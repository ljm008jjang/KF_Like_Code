// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Monster.h"
#include "Animation/AnimInstance.h"
#include "MonsterAnimInstance.generated.h"

class AMonster;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UMonsterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

	virtual void NativeInitializeAnimation() override;

private:
	UPROPERTY()
	TSoftObjectPtr<AMonster> Monster;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintGetter=GetIsHeadless, BlueprintSetter=SetIsHeadless)
	bool IsHeadless = false;

	UFUNCTION(BlueprintGetter)
	bool GetIsHeadless()
	{
		return IsHeadless;
	}

	UFUNCTION(BlueprintSetter)
	void SetIsHeadless(bool NewIsHeadless)
	{
		IsHeadless = NewIsHeadless;
	}

	UFUNCTION(BlueprintGetter)
	bool GetIsNotHeadless()
	{
		return !IsHeadless;
	}

	UFUNCTION(BlueprintPure)
	bool GetIsDead()
	{
		if (Monster.Get() == nullptr)
		{
			return false;
		}
		return Monster.Get()->IsDead();
	}

	UFUNCTION(BlueprintPure)
	bool GetIsNotDead()
	{
		if (Monster.Get() == nullptr)
		{
			return false;
		}
		return Monster.Get()->IsDead() == false;
	}
};
