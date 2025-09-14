// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "UnitManager.generated.h"

enum class EMonsterType : uint8;

UCLASS()
class KILLINGFLOORLIKE_API AUnitManager : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AUnitManager();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	//current wave max spawn monster num
	UPROPERTY(ReplicatedUsing=OnRep_RemainMonsterCount)
	int32 MaxSpawnedMonsters;
	void SetMaxSpawnedMonsters(int32 NewMaxSpawnedMonsters)
	{
		if (HasAuthority())
		{
			MaxSpawnedMonsters = NewMaxSpawnedMonsters;
			OnRep_RemainMonsterCount();
		}
	}
	//현재 전체 게임에서 스폰될 + 살아있는 몬스터 개수
	UPROPERTY(ReplicatedUsing=OnRep_RemainMonsterCount)
	int32 RemainMonsterCount;

	void SetRemainMonsterCount(int32 NewRemainMonsterCount)
	{
		if (HasAuthority())
		{
			RemainMonsterCount = NewRemainMonsterCount;
			OnRep_RemainMonsterCount();
		}
	}

	UFUNCTION()
	void OnRep_RemainMonsterCount();


	UPROPERTY(EditAnywhere)
	TMap<EMonsterType, int32> MonsterPool;

	UPROPERTY(EditAnywhere)
	TArray<AActor*> SpawnPoints;

	UPROPERTY(EditAnywhere)
	TMap<EMonsterType, TSubclassOf<class AMonster>> MonsterClass;

	UPROPERTY(VisibleAnywhere)
	TArray<AMonster*> Monsters;

public:
	void SpawnMonster();
	void SpawnMonster(EMonsterType SelectedMonster);

	UFUNCTION(BlueprintCallable)
	int GetSpawnedMonsterCount(bool IsAlive);

	void ClearUnitDB();

	TArray<AActor*> GetSpawnPoints();

	void RefillMonsterPool(const int32 CurrentWave);
	int32 GetMaxSpawnedMonsters();

	void HandleMonsterDeath()
	{
		SetRemainMonsterCount(RemainMonsterCount - 1);
	}

	int32 GetRemainMonsterCount()
	{
		return RemainMonsterCount;
	}
};
