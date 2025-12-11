// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PoolableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPoolableStateChanged);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class KILLINGFLOORLIKE_API UPoolableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UPoolableComponent();

	// 이 변수가 복제될 때마다 OnRep_IsActive 함수를 호출합니다.
	UPROPERTY(ReplicatedUsing = OnRep_IsPooling)
	bool IsPooling;
	void SetIsPooling(bool NewIsPooling)
	{
		if (IsValid(GetOwner()) && GetOwner()->HasAuthority())
		{
			IsPooling = NewIsPooling;
			OnRep_IsPooling();
		}
	}

	// RepNotify 함수 선언
	UFUNCTION()
	void OnRep_IsPooling();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// 풀 매니저가 호출할 함수들
	void ActivateFromPool();
	void DeactivateToPool();

	// ▼▼▼▼▼ 콜백을 위한 델리게이트 선언 ▼▼▼▼▼
	UPROPERTY(BlueprintAssignable, Category = "Pooling")
	FOnPoolableStateChanged OnActivated;

	UPROPERTY(BlueprintAssignable, Category = "Pooling")
	FOnPoolableStateChanged OnDeactivated;
	// ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲
};
