// Fill out your copyright notice in the Description page of Project Settings.


#include "ObjectPoolManager.h"

#include "PoolableComponent.h"
#include "Kismet/GameplayStatics.h"


void UObjectPoolManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// 레벨이 로드되기 직전에 호출될 델리게이트에 ClearAllPools 함수를 바인딩합니다.
	OnPreLoadMapHandle = FCoreUObjectDelegates::PreLoadMap.AddUObject(this, &UObjectPoolManager::ClearAllPools);
}

void UObjectPoolManager::Deinitialize()
{
	FCoreUObjectDelegates::PreLoadMap.Remove(OnPreLoadMapHandle);
	Super::Deinitialize();
}

UObjectPoolManager* UObjectPoolManager::GetObjectPoolManager(UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;

	UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	if (!GameInstance) return nullptr;

	return GameInstance->GetSubsystem<UObjectPoolManager>();
}

void UObjectPoolManager::InitializePool(const TSubclassOf<AActor>& ActorClass, const int32 InitialSize)
{
	if (!ActorClass) return;

	UWorld* World = GetWorld();
	if (!World) return;

	FActorPoolList& Pool = PoolMap.FindOrAdd(ActorClass);

	// 불필요한 메모리 재할당을 피하기 위해 배열의 크기를 미리 예약합니다.
	Pool.PooledActors.Reserve(Pool.PooledActors.Num() + InitialSize);

	for (int32 i = 0; i < InitialSize; ++i)
	{
		AActor* NewActor = World->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator);
		if (NewActor)
		{
			// PoolableComponent의 비활성화 로직을 사용하여 일관된 상태로 만듭니다.
			if (UPoolableComponent* PoolableComp = NewActor->GetComponentByClass<UPoolableComponent>())
			{
				PoolableComp->DeactivateToPool();
			}
			Pool.PooledActors.Add(NewActor);
		}
	}
}

AActor* UObjectPoolManager::GetFromPoolBP(UObject* WorldContextObject, const TSubclassOf<AActor> ActorClass,
                                          const FVector Location,
                                          const FRotator Rotation)
{
	return GetFromPool(WorldContextObject->GetWorld(), ActorClass, Location, Rotation, FActorSpawnParameters());
}

AActor* UObjectPoolManager::GetFromPool(UWorld* World, const TSubclassOf<AActor>& ActorClass, const FVector& Location,
                                        const FRotator& Rotation, const FActorSpawnParameters& ActorSpawnParams)
{
	if (!ActorClass) return nullptr;

	// GetFromPool은 서버에서만 호출되어야 합니다.
	// 참고: GameInstanceSubsystem은 서버와 클라이언트에 각각 존재하므로,
	// 이 풀은 서버의 풀이며 클라이언트의 풀과는 동기화되지 않습니다.
	if (!World)
	{
		UE_LOG(LogTemp, Warning, TEXT("GetFromPool should only be called on the server!"));
		return nullptr;
	}

	FActorPoolList& Pool = PoolMap.FindOrAdd(ActorClass);
	AActor* ActorToUse = nullptr;

	// 풀에 남은 게 있으면 재사용
	if (!Pool.PooledActors.IsEmpty())
	{
		ActorToUse = Pool.PooledActors.Pop();

		// 레벨 전환 등으로 인해 풀에 있던 액터가 파괴되었을 수 있으므로, 유효성을 검사합니다.
		if (!IsValid(ActorToUse))
		{
			// 액터가 유효하지 않으면, 풀의 다른 액터를 사용하기 위해 재귀 호출합니다.
			// 풀의 크기가 줄어들기 때문에 무한 루프에 빠지지 않습니다.
			return GetFromPool(World, ActorClass, Location, Rotation, ActorSpawnParams);
		}
	}
	else // 없으면 새로 생성
	{
		ActorToUse = World->SpawnActor<AActor>(ActorClass, Location, Rotation, ActorSpawnParams);
	}

	if (ActorToUse)
	{
		ActorToUse->SetActorLocation(Location, false, nullptr, ETeleportType::TeleportPhysics);
		ActorToUse->SetActorRotation(Rotation, ETeleportType::TeleportPhysics);

		if (UPoolableComponent* PoolableComp = ActorToUse->GetComponentByClass<UPoolableComponent>())
		{
			PoolableComp->ActivateFromPool();
		}
	}

	return ActorToUse;
}

void UObjectPoolManager::ReturnToPool(AActor* Actor)
{
	// 서버만 풀을 관리해야 하며, 액터가 유효한지 확인합니다.
	if (!IsValid(Actor) || !Actor->HasAuthority())
	{
		return;
	}

	// PoolableComponent가 상태 전환(숨김, 콜리전 비활성화 등)을 책임집니다.
	if (UPoolableComponent* PoolableComp = Actor->GetComponentByClass<UPoolableComponent>())
	{
		PoolableComp->DeactivateToPool();
	}

	TSubclassOf<AActor> ClassType = Actor->GetClass();
	FActorPoolList& Pool = PoolMap.FindOrAdd(ClassType);

	Pool.PooledActors.Add(Actor);
}

void UObjectPoolManager::ReturnToPoolAfterDelay(AActor* Actor, FTimerHandle* TimerHandle, const float Delay)
{
	// 타이머를 설정하기 전에 액터와 월드가 유효한지 확인합니다.
	if (!Actor || !Actor->GetWorld()) return;

	// 참고: 타이머 핸들을 포인터로 전달하는 것은 일반적이지 않으며, 호출자가 핸들의 생명주기를 책임져야 합니다.
	Actor->GetWorld()->GetTimerManager().SetTimer(*TimerHandle,
	                                              FTimerDelegate::CreateUObject(
		                                              this, &UObjectPoolManager::ReturnToPool, Actor), Delay, false);
}

void UObjectPoolManager::ClearAllPools(const FString& MapName)
{
	UE_LOG(LogTemp, Log, TEXT("UObjectPoolManager::ClearAllPools - Clearing all object pools due to level change."));
	// 레벨이 언로드되면 그 안에 있던 모든 액터는 파괴됩니다.
	// 다음 레벨 로드 시 문제가 발생하지 않도록, 유효하지 않게 된 포인터들을 담고 있는 맵을 비워줍니다.
	PoolMap.Empty();
}
