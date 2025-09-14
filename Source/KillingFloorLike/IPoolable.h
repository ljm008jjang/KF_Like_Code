// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "IPoolable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UIPoolable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class KILLINGFLOORLIKE_API IIPoolable
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	// 풀에서 꺼낼 때 호출 (활성화된 상태로 초기화)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Pooling")
	void OnPooledActivate();

	// 풀에 반환될 때 호출 (상태 초기화)
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category="Pooling")
	void OnPooledReset();

	//virtual void DelayReturn(float DelayTime);
};
