// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundManager.h"

#include "BaseCharacter.h"
#include "SoundManagerActor.h"
#include "Kismet/GameplayStatics.h"

USoundManager* USoundManager::GetSoundManager(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance) return nullptr;

	return GameInstance->GetSubsystem<USoundManager>();
}

void USoundManager::Init()
{
	UWorld* World = GetWorld();

	if (World && World->GetAuthGameMode())
	{
		UE_LOG(LogTemp, Warning, TEXT("SoundActor Spawned on Server: %s"), *World->GetName());
		SoundActor = World->SpawnActor<ASoundManagerActor>();
	}
}


void USoundManager::Play2DSound(USoundBase* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
}

void USoundManager::Multi_Play3DSound(USoundBase* Sound, FVector Location)
{
	UWorld* World = GetWorld();
	if (World == nullptr || World->GetAuthGameMode() == nullptr)
	{
		return;
	}
	if (SoundActor && Sound)
	{
		SoundActor->Multi_Play3DSound(Sound, Location);
	}
}

void USoundManager::Server_Play3DSoundAttached_Implementation(USoundBase* Sound, ABaseCharacter* Character)
{
	if (Sound && Character)
	{
		// 캐릭터의 멀티캐스트 함수를 호출하되, '사운드 애셋'을 넘겨줍니다.
		Character->Multicast_PlayAndSetSound(Sound);
	}
}

void USoundManager::Server_Stop3DSoundAttached_Implementation(ABaseCharacter* Character)
{
	if (Character)
	{
		Character->Multi_StopAudioComponent();
	}
}

void USoundManager::Multi_ChangeBGM(const FString& AssetPath)
{
	UWorld* World = GetWorld();
	if (World == nullptr || World->GetAuthGameMode() == nullptr)
	{
		return;
	}
	if (SoundActor)
	{
		SoundActor->Multi_ChangeBGM(AssetPath);
	}
}
