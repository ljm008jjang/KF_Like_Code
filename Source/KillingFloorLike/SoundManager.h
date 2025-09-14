// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SoundManager.generated.h"

class ABaseCharacter;
class ASoundManagerActor;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API USoundManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	//RPC함수 실행 위한 actor
	UPROPERTY()
	ASoundManagerActor* SoundActor;


public:
	UFUNCTION(BlueprintCallable, Category="Subsystem")
	static USoundManager* GetSoundManager(UObject* WorldContextObject);
	
	void Init();
	UFUNCTION(BlueprintCallable)
	void Play2DSound(USoundBase* Sound);

	/** 위치 기반 사운드 재생 */
	UFUNCTION(BlueprintCallable)
	void Multi_Play3DSound(USoundBase* Sound, FVector Location);
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_Play3DSoundAttached(USoundBase* Sound, ABaseCharacter* Character);
	UFUNCTION(Server, Reliable, BlueprintCallable)
	void Server_Stop3DSoundAttached(ABaseCharacter* Character);
	void Multi_ChangeBGM(const FString& AssetPath);
};
