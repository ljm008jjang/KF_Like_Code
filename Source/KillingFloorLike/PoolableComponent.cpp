// Fill out your copyright notice in the Description page of Project Settings.


#include "PoolableComponent.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
UPoolableComponent::UPoolableComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
	// ...
}

void UPoolableComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(UPoolableComponent, IsPooling);
}

// 공통 로직을 여기에 딱 한 번만 작성합니다.
void UPoolableComponent::OnRep_IsPooling()
{
	AActor* Owner = GetOwner();
	if (IsValid(Owner) == false) return;
	if (IsPooling == false)
	{
		// '비활성화' 이벤트를 구독한 모든 함수를 호출합니다.
		OnDeactivated.Broadcast();
	}

	Owner->SetActorHiddenInGame(!IsPooling);
	Owner->SetActorEnableCollision(IsPooling);
	Owner->SetActorTickEnabled(IsPooling);

	// 캐릭터인지 확인합니다.
	ACharacter* OwnerCharacter = Cast<ACharacter>(Owner);
	if (OwnerCharacter)
	{
		UCharacterMovementComponent* MovementComp = OwnerCharacter->GetCharacterMovement();
		if (MovementComp)
		{
			if (IsPooling)
			{
				// 활성화될 때: 컴포넌트를 활성화하고, 이동 모드를 Walking으로 설정
				MovementComp->Activate();
				MovementComp->SetMovementMode(EMovementMode::MOVE_Walking);
			}
			else
			{
				// 비활성화될 때: 이동을 멈추고, 컴포넌트를 비활성화
				MovementComp->StopMovementImmediately();
				MovementComp->SetMovementMode(EMovementMode::MOVE_None);
				MovementComp->Deactivate();
			}
		}
	}

	if (UProjectileMovementComponent* ProjectileMovement = Owner->FindComponentByClass<UProjectileMovementComponent>())
	{
		//ProjectileMovement->physic
	}else if (UStaticMeshComponent* MeshComponent = Owner->FindComponentByClass<UStaticMeshComponent>()){
		if (IsPooling) // 활성화 될 때
		{
			// ✅ 2. 그 다음에 물리 시뮬레이션을 켭니다.
			MeshComponent->SetSimulatePhysics(IsPooling);
		}
		else // 비활성화 될 때
		{
			// ✅ 1. 물리 시뮬레이션을 먼저 끕니다.
			MeshComponent->SetSimulatePhysics(IsPooling);
		}
	}
	    
	if (IsPooling)
	{
		// '활성화' 이벤트를 구독한 모든 함수를 호출합니다.
		OnActivated.Broadcast();
	}
}

void UPoolableComponent::ActivateFromPool()
{
	SetIsPooling(true);
}

void UPoolableComponent::DeactivateToPool()
{
	SetIsPooling(false);
}
