// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SoundManagerActor.generated.h"

UCLASS()
class KILLINGFLOORLIKE_API ASoundManagerActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASoundManagerActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;


private:
	UPROPERTY()
	UAudioComponent* BGMAudioComponent;

	UPROPERTY()
	FTimerHandle TimerHandle;
	
public:
	/** 위치 기반 사운드 재생 */
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_Play3DSound(USoundBase* Sound, FVector Location);
	
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable)
	void Multi_ChangeBGM(const FString& AssetPath);
};
