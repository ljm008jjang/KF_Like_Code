// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "DebugManagerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KILLINGFLOORLIKE_API UDebugManagerComponent : public USceneComponent
{
	GENERATED_BODY()

public:

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:

private:
	inline static bool IsDebugActive = false;

public:
	static void DrawDebugLineManager(const UWorld* InWorld, FVector const& LineStart, FVector const& LineEnd,
	                                 FColor const& Color, bool bPersistentLines = false, float LifeTime = -1.f,
	                                 uint8 DepthPriority = 0, float Thickness = 0.f);
	static void DrawDebugSphereManager(const UWorld* InWorld, FVector const& Center,
	                                   float const Radius, FColor const& Color, bool bPersistentLines,
	                                   float LifeTime = -1.f,
	                                   uint8 DepthPriority = 0, float Thickness = 0.f);
};
