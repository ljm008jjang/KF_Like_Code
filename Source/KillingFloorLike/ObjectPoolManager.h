// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ObjectPoolManager.generated.h"

USTRUCT(BlueprintType)
struct FActorPoolList
{
	GENERATED_BODY()

	// 풀에 들어 있는 액터들
	UPROPERTY()
	TArray<AActor*> PooledActors;
};

UCLASS()
class KILLINGFLOORLIKE_API UObjectPoolManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	UFUNCTION(BlueprintCallable, Category="Subsystem")
	static UObjectPoolManager* GetObjectPoolManager(UObject* WorldContextObject);
	
	// 풀 생성
	void InitializePool(const TSubclassOf<AActor>& ActorClass, int32 InitialSize);
	
	// 블루프린트용
	UFUNCTION(BlueprintCallable)
	AActor* GetFromPoolBP(UObject* WorldContextObject, TSubclassOf<AActor> ActorClass, FVector Location, FRotator Rotation);
	
	AActor* GetFromPool(UWorld* World, const TSubclassOf<AActor>& ActorClass, const FVector& Location, const FRotator& Rotation,
	                    const FActorSpawnParameters& ActorSpawnParams);

	// 풀에서 꺼내오기, 템플릿 함수는 컴파일 타임에서 코드가 생성되므로, 헤더파일에서 구현 필요
	template <typename T>
	T* GetFromPoolTemplate(UWorld* World, TSubclassOf<AActor> ActorClass, FVector Location, FRotator Rotation,
	                       FActorSpawnParameters ActorSpawnParams);

	// 풀에 반환하기
	void ReturnToPool(AActor* Actor);
	void ReturnToPoolAfterDelay(AActor* Actor, FTimerHandle* TimerHandle, float Delay);


private:
	// 풀 저장소: ActorClass → List
	UPROPERTY()
	TMap<TSubclassOf<AActor>, FActorPoolList> PoolMap;
	
	// 레벨이 변경될 때 풀을 정리하기 위한 델리게이트 핸들입니다.
	FDelegateHandle OnPreLoadMapHandle;
 
	// 모든 풀의 내용을 비웁니다.
	UFUNCTION()
	void ClearAllPools(const FString& MapName);
};

template <typename T>
T* UObjectPoolManager::GetFromPoolTemplate(UWorld* World, const TSubclassOf<AActor> ActorClass, const FVector Location,
                                           const FRotator Rotation,
                                           const FActorSpawnParameters ActorSpawnParams)
{
	return Cast<T>(GetFromPool(World, ActorClass, Location, Rotation, ActorSpawnParams));
}
