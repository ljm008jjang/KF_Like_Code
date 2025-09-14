// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "KillingFloorLikeGameState.generated.h"

class AShopManager;
class AUnitManager;


UENUM(BlueprintType)
enum class EModeType : uint8
{
	None,
	Wave,
	Break,
	End
};

// UI 업데이트를 위한 델리게이트 선언 (이전과 동일)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUIUpdateDelegate);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnUIMonsterCountUpdateDelegate, int32, RemainingMonstersCount);

// UI 업데이트를 위한 델리게이트 선언 (이전과 동일)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTimeChangedSignature);

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API AKillingFloorLikeGameState : public AGameStateBase
{
	GENERATED_BODY()

	virtual void BeginPlay() override;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintGetter = GetUnitManager)
	AUnitManager* UnitManager;

	UPROPERTY(EditDefaultsOnly, BlueprintGetter = GetShopManager)
	AShopManager* ShopManager;

	UPROPERTY(ReplicatedUsing = OnRep_CurrentModeType, BlueprintGetter = GetCurrentModeType)
	EModeType CurrentModeType = EModeType::None;
	UFUNCTION()
	void OnRep_CurrentModeType();
	UPROPERTY(Replicated)
	bool IsWin = false;

	UPROPERTY(ReplicatedUsing=OnRep_CurrentWave)
	int32 CurrentWave = 0;
	UFUNCTION()
	void OnRep_CurrentWave();

	// 클라이언트에서만 재생되어야 하는 사운드 에셋을 미리 로드해두면 좋습니다.
	UPROPERTY()
	TObjectPtr<USoundBase> ZedTimeStartSound;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(ReplicatedUsing=OnRep_BreakEndTime)
	float BreakEndTime;
	// BreakTimeRemaining 값이 복제되었을 때 클라이언트에서 호출될 함수
	UFUNCTION()
	void OnRep_BreakEndTime();

	void SetBreakEndTime(float NewBreakEndTime)
	{
		if (HasAuthority())
		{
			BreakEndTime = NewBreakEndTime;
			OnRep_BreakEndTime();
		}
	}

	//UPROPERTY(BlueprintGetter=GetGameStartBreakTime)
	int32 GameStartBreakTime = 5;
	int32 MaxBreakTime = 10;

	float GetBreakEndTime() const
	{
		return BreakEndTime;
	}

	// UI 업데이트를 위한 델리게이트 (이벤트를 방송할 용도)
	FOnTimeChangedSignature OnEndTimeChanged;


	UPROPERTY(Replicated)
	int32 MaxWaveIndex = 10;

	int32 GetCurrentWave() const
	{
		return CurrentWave;
	}

	void SetCurrentWave(int32 NewCurrentWave)
	{
		if (HasAuthority())
		{
			this->CurrentWave = NewCurrentWave;
			OnRep_CurrentWave();
		}
	}

	// 시간 딜레이션 값이 변경되면 OnRep_CurrentTimeDilation 함수를 클라이언트에서 호출합니다.
	UPROPERTY(ReplicatedUsing = OnRep_CurrentTimeDilation)
	float CurrentTimeDilation;

	UFUNCTION()
	void OnRep_CurrentTimeDilation();
	//void SetBreakTimeRemaining(int32 NewTime);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnUIUpdateDelegate OnGameStartUIUpdate;

	/*UFUNCTION(BlueprintCallable)
	int32 GetWaveRemainMonsterCount();*/
	UFUNCTION(BlueprintCallable)
	FText GetWaveText();

	UFUNCTION(BlueprintGetter)
	EModeType GetCurrentModeType() const
	{
		return CurrentModeType;
	}

	void SetCurrentModeType(EModeType ModeType)
	{
		if (HasAuthority())
		{
			CurrentModeType = ModeType;
			OnRep_CurrentModeType();
		}
	}
	void SetIsWin(bool NewIsWin)
	{
		if (HasAuthority())
		{
			IsWin = NewIsWin;
		}
	}

	UFUNCTION(BlueprintGetter)
	AUnitManager* GetUnitManager()
	{
		return UnitManager;
	}

	UFUNCTION(BlueprintGetter)
	AShopManager* GetShopManager()
	{
		return ShopManager;
	}
};
