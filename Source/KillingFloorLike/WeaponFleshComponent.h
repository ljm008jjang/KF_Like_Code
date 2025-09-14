// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "WeaponFleshComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KILLINGFLOORLIKE_API UWeaponFleshComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UWeaponFleshComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


private:
	UPROPERTY()
	class USpotLightComponent* FleshLight;

	UPROPERTY(EditAnywhere, Blueprintable, BlueprintGetter=GetFleshAnimMontage)
	UAnimMontage* FleshAnimMontage;

public:
	// 플래시라이트 On/Off 함수
	UFUNCTION(BlueprintCallable)
	void ToggleFlashlight(bool IsFleshOn);

	UFUNCTION(BlueprintCallable)
	bool GetIsFleshOn();

	UFUNCTION(BlueprintCallable, BlueprintGetter)
	UAnimMontage* GetFleshAnimMontage();
};
