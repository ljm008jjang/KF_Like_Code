// Fill out your copyright notice in the Description page of Project Settings.


#include "SoundManagerActor.h"

#include "ResourceManager.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
ASoundManagerActor::ASoundManagerActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

// Called when the game starts or when spawned
void ASoundManagerActor::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(BGMAudioComponent) == false)
	{
		BGMAudioComponent = NewObject<UAudioComponent>(this);
		BGMAudioComponent->bAutoActivate = false; // 자동재생 끔
		BGMAudioComponent->bIsUISound = true; // UI/배경음악용
		BGMAudioComponent->RegisterComponent();
	}
}

void ASoundManagerActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
}

void ASoundManagerActor::Multi_Play3DSound_Implementation(USoundBase* Sound, FVector Location)
{
	UGameplayStatics::PlaySoundAtLocation(GetWorld(), Sound, Location);
}

void ASoundManagerActor::Multi_ChangeBGM_Implementation(const FString& AssetPath)
{
	if (IsValid(BGMAudioComponent) == false)
	{
		return;
	}

	float FadeDuration = 2;
	BGMAudioComponent->FadeOut(FadeDuration, 0.0f);
	// Delay 후 새 사운드 재생 (Fade In)

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [=, this]()
	{
		BGMAudioComponent->
			SetSound(GetGameInstance()->GetSubsystem<UResourceManager>()->LoadSound(AssetPath));
		BGMAudioComponent->FadeIn(FadeDuration, 1.0f); // Target Volume = 1.0
	}, FadeDuration, false);
}
