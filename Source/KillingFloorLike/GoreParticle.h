// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoreParticle.generated.h"

class UPoolableComponent;

UCLASS()
class KILLINGFLOORLIKE_API AGoreParticle : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AGoreParticle();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	

protected:
	virtual void PostInitializeComponents() override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPoolableComponent* PoolableComponent;

public:
	UPROPERTY(EditAnywhere)
	TArray<UStaticMesh*> BrainParticleStaticMeshes;
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* StaticMeshComponent;

	UFUNCTION(NetMulticast, Unreliable)
	void Multi_AddImpurse(FVector HeadLocation);

	// 복제된 메시 인덱스를 저장할 변수
	UPROPERTY(ReplicatedUsing = OnRep_CurrentMeshIndex)
	int32 CurrentMeshIndex;

private:
	void SetCurrentMeshIndex(int32 NewCurrentMeshIndex)
	{
		if (HasAuthority())
		{
			CurrentMeshIndex = NewCurrentMeshIndex;
			OnRep_CurrentMeshIndex();
		}
	}
	UFUNCTION()
	void OnRep_CurrentMeshIndex();

	UFUNCTION()
	void OnPooledActivate();

	FTimerHandle TimerHandle;
};
