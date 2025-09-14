// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PubSubManager.h"
#include "GameFramework/GameModeBase.h"
#include "KillingFloorLikeGameMode.generated.h"

enum class EModeType : uint8;
class AKillingFloorLikeGameState;
class AShopManager;
class UDebugManagerComponent;
enum class EMonsterType : uint8;
class AUnitManager;

UCLASS(minimalapi)
class AKillingFloorLikeGameMode : public AGameModeBase
{
	GENERATED_BODY()

private:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	AKillingFloorLikeGameMode();

private:
	int32 MaxWaveTime = 3;
	int32 WaveDelayTime = 0;

	float BreakDuration = 60.0f;
	float GameEndDuration = 5.0f;

	FTimerHandle GameStartTimerHandle;
	FTimerHandle CountdownTimerHandle;
	FTimerHandle GameEndTimerHandle;
	FTimerHandle ZedTimerHandle;

	// 캐싱해둘 GameState 포인터
	UPROPERTY(BlueprintGetter = GetCachedGameState) // UPROPERTY로 GC(가비지 컬렉션)가 임의로 메모리를 해제하는 것을 방지
	AKillingFloorLikeGameState* CachedGameState;

	/*UPROPERTY()
	class UAudioComponent* BGMComponent;*/

	UPROPERTY()
	class UNavigationSystemV1* NavSystem;

	//UDebugManagerComponent* DebugManagerComponent;

	UFUNCTION(BlueprintCallable)
	void StartWave();
	void UpdateWaveTimeCountdown();
	//void SpawnMonster();
	void CheckWaveStateMonsterCount(FGameplayTag Channel, const FGameplayMessage_MonsterCount& Message);
	void CheckWaveStatePlayerDead(FGameplayTag Channel, const FGameplayMessage_None& Message);
	void EndWave(bool IsWin);

	//UFUNCTION(Server, )
	void StartBreak();
	void EndBreak();

	void EndMatch(bool IsWin);

	void ChangeModeType(EModeType NewModeType);

	// Check and Refill Pool Function
	//void RefillMonsterPool();


	//void ChangeBGM(const FString& AssetPath);

	//void FindPath(const FVector& Start,const FVector& Goal);
	void SetAllPlayersChangablePerk(bool bCanChange);

public:
	/*UFUNCTION(BlueprintCallable)
	int32 GetWaveRemainMonsterCount();*/

	/*UFUNCTION(Category = "GameMode Events")
	void OnUpdateRemainMonsterCount();*/

	UFUNCTION(Category = "GameMode Events")
	void OnStartBreakEvent();
	UFUNCTION(Category = "GameMode Events")
	void OnGameStartEvent(float BreakDurationVal = 60.0f);

	/*UFUNCTION(BlueprintPure, BlueprintGetter)
	EModeType GetCurrentModeType();*/


	void StartZedTime(const float Duration, const float SlowRate);
	void StopZedTime();

	UFUNCTION(BlueprintGetter)
	AKillingFloorLikeGameState* GetCachedGameState() const
	{
		return CachedGameState;
	}

	void UpdateSessionWaveInfo(int32 NewWave);
};
