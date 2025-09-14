// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "PubSubManager.h"
#include "Blueprint/UserWidget.h"
#include "CrossHair.generated.h"

class UCanvasPanel;
class UPerkIconWithStarWidget;
class UImage;
class UWidgetSwitcher;
class AKillingFloorLikeGameState;
class UTextBlock;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UCrossHair : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

private:
	//~==============================================================================================
	// Cached Pointers
	//~==============================================================================================

	UPROPERTY()
	AKillingFloorLikeGameState* GameState;

	UPROPERTY()
	class APlayerCharacterController* PlayerCharacterController;
	UPROPERTY()
	class AKillingFloorLikeCharacter* OwningPlayerCharacter;

	//~==============================================================================================
	// Widgets
	//~==============================================================================================

	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* OverallSwitcher;
	UPROPERTY(meta=(BindWidget))
	UCanvasPanel* MainCanvas;
	UPROPERTY(meta=(BindWidget))
	UImage* WipedOutImage;
	UPROPERTY(meta=(BindWidget))
	UImage* SurvivedImage;


	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* InfoSwitcher;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* MonsterText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WaveText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* BreakTimeText;
	FTimerHandle CountdownTimerHandle;

	UPROPERTY(meta=(BindWidget))
	UImage* HitSerious;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* HpText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ArmorText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* TradeText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ShopDistText;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* BulletText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* ClipText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* MoneyText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeightText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* GranadeText;

	UPROPERTY(meta=(BindWidget))
	UPerkIconWithStarWidget* PerkIconWithStar;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* HitAnimation;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* VomitAnimation;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* WaveInboundAnimation;

	//~==============================================================================================
	// Initialization
	//~==============================================================================================

	/** Caches pointers to frequently accessed actors like GameState and PlayerCharacter. */
	void CacheActorPointers();

	/** Subscribes all necessary UI update functions to the PubSubManager. */
	void SubscribeToEvents();

	//~==============================================================================================
	// Event Handlers (Called by PubSubManager)
	//~==============================================================================================

	void OnEndMatch(FGameplayTag Channel, const FGameplayMessage_EndMatch& Message);
	void OnMonsterCountChanged(FGameplayTag Channel, const FGameplayMessage_MonsterCount& Message);
	void OnGameModeChanged(FGameplayTag Channel, const FGameplayMessage_GameMode& Message);
	void OnBreakEndTimeChanged(FGameplayTag Channel, const FGameplayMessage_BreakEndTimeChanged& Message);
	void OnPlayerHealthChanged(FGameplayTag Channel, const FGameplayMessage_PlayerHeathChange& Message);
	void OnPlayerAmmoChanged(FGameplayTag Channel, const FGameplayMessage_PlayerAmmoChange& Message);
	void OnPlayerMoneyChanged(FGameplayTag Channel, const FGameplayMessage_PlayerMoneyChange& Message);
	void OnPlayerWeightChanged(FGameplayTag Channel, const FGameplayMessage_None& Message);

	//~==============================================================================================
	// UI Update Logic
	//~==============================================================================================

	void UpdateMonsterCountText(int32 RemainMonsterCount) const;
	void SwitchInfoDisplay(EModeType CurrentModeType);
	void StartBreakdownTimer(int32 BreakEndTime);
	void UpdateWaveText() const;
	void UpdateBreakdownTimer(int32 EndTime);
	void UpdateHealthAndArmorText(int32 Health, int32 Armor) const;
	void UpdateShopDistanceText() const;
	void UpdateAmmoAndGrenadeText() const;
	void UpdateMoneyText(int32 Money) const;
	void UpdateWeightText() const;

	//~==============================================================================================
	// Animations
	//~==============================================================================================

	UFUNCTION(BlueprintCallable, Category = "Animation")
	void PlayWaveInboundAnimation();

public :
	void UpdateTradeTextVisibility(bool bIsVisible) const;
	void Client_PlayHitAnimation(TSubclassOf<UDamageType> DamageType, float CurrentHp);
};
