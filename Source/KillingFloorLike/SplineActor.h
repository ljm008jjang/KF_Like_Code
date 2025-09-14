// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SplineActor.generated.h"

class AKillingFloorLikeGameState;

UCLASS()
class KILLINGFLOORLIKE_API ASplineActor : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ASplineActor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION(BlueprintCallable)
	void CreatePathTo();

private:
	UPROPERTY()
	class USplineComponent* SplineComponent;

	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* NiagaraEffect;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* NiagaraSystem;

	UPROPERTY()
	ACharacter* Character;

	// 캐싱해둘 GameState 포인터
	UPROPERTY() // UPROPERTY로 GC(가비지 컬렉션)가 임의로 메모리를 해제하는 것을 방지
	TObjectPtr<AKillingFloorLikeGameState> CachedGameState;

	FTimerHandle MoveTimerHandle;
	float MoveTime;
	float MoveDuration;
};
