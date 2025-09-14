// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPoolable.h"
#include "GameFramework/Actor.h"
#include "MoneyObject.generated.h"

class UPoolableComponent;
class UTP_PickUpComponent;

UCLASS()
class KILLINGFLOORLIKE_API AMoneyObject : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMoneyObject();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPoolableComponent* PoolableComponent;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMeshComponent;
	UPROPERTY()
	UTP_PickUpComponent* PickUpComponent;

	UPROPERTY(EditAnywhere)
	int32 MoneyAmount;

	/** Attaches the actor to a FirstPersonCharacter */
	UFUNCTION(BlueprintCallable)
	void PickUpMoney(class AKillingFloorLikeCharacter* TargetCharacter);

	UFUNCTION()
	void OnPooledActivate();
	UFUNCTION()
	void OnPooledReset();

public:
	void SetMoneyAmount(int32 Amount);
};
