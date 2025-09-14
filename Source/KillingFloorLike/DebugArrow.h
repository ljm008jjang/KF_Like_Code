// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DebugArrow.generated.h"

UCLASS()
class KILLINGFLOORLIKE_API ADebugArrow : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ADebugArrow();
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UStaticMeshComponent* ArrowMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneCaptureComponent2D* SceneCaptureComponent;
private:
	UPROPERTY()
	class AShopManager* ShopManager;
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> OwningController;
public:

	void Init(AShopManager* NewShopManager);
};
