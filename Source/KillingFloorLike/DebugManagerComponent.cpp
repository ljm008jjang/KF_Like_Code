// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugManagerComponent.h"


// Called when the game starts
void UDebugManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	IsDebugActive = false;
	// ...
}

void UDebugManagerComponent::DrawDebugLineManager(const UWorld* InWorld, FVector const& LineStart,
                                                  FVector const& LineEnd,
                                                  FColor const& Color, bool bPersistentLines, float LifeTime,
                                                  uint8 DepthPriority, float Thickness)
{
	if (IsDebugActive == false)
	{
		return;
	}

	DrawDebugLine(InWorld, LineStart, LineEnd, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
}

void UDebugManagerComponent::DrawDebugSphereManager(const UWorld* InWorld, FVector const& Center,
                                                    float const Radius, FColor const& Color, bool bPersistentLines,
                                                    float LifeTime, uint8 DepthPriority,
                                                    float Thickness)
{
	if (IsDebugActive == false)
	{
		return;
	}

	DrawDebugSphere(InWorld, Center, Radius, 32, Color, bPersistentLines, LifeTime, DepthPriority, Thickness);
}
