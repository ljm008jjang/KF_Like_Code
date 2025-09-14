// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ShopManager.generated.h"

class UShopComponent;

UCLASS()
class KILLINGFLOORLIKE_API AShopManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AShopManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

private:
	UPROPERTY(EditAnywhere)
	TArray<AActor*> ShopComponents;

	UPROPERTY(Replicated, EditAnywhere, BlueprintGetter=GetCurrentShop)
	class AActor* CurrentShop;
	
	UPROPERTY(EditAnywhere)
	class ASplineActor* SplineActor;

	UPROPERTY()
	ACharacter* Character;

public:
	void SetNewShop();

	UFUNCTION(BlueprintGetter)
	AActor* GetCurrentShop() const;

	UFUNCTION(BlueprintCallable)
	FText GetShopDistText();
};
