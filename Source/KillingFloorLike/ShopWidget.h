// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KillingFloorLikeCharacter.h"
#include "PubSubManager.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "ShopWidget.generated.h"

class UShopArmorWidget;
class UBasicButtonWidget;
class UPerkIconWithStarWidget;
class UShopQuickPerkSelectWidget;
class UShopPerkImageWidget;
class UVerticalBox;
class UTextBlock;
class UImage;
class UShopPurchaseWeaponWidget;
class UScrollBox;
class UShopPlayerWeaponWidget;
class ABaseWeapon;

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UShopWidget : public UUserWidget
{
	GENERATED_BODY()

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual bool Initialize() override;

private:
	UPROPERTY(BlueprintGetter = GetPlayerCharacterController)
	class APlayerCharacterController* PlayerCharacterController;

	UPROPERTY(BlueprintGetter=GetShopPlayerWeaponArray)
	TArray<UShopPlayerWeaponWidget*> ShopPlayerWeaponArray;


	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UShopPlayerWeaponWidget> ShopPlayerWeaponWidgetClass;

	UPROPERTY()
	UShopPlayerWeaponWidget* SelectedShopPlayerWidget;
	UPROPERTY(meta=(BindWidget))
	UShopPlayerWeaponWidget* KnifeWeaponWidget;
	UPROPERTY(meta=(BindWidget))
	UShopPlayerWeaponWidget* GrenadesWeaponWidget;
	UPROPERTY(meta=(BindWidget))
	UShopArmorWidget* CombatArmorWidget;

	UPROPERTY(meta=(BindWidget))
	UScrollBox* PlayerWeaponScroll;

	UPROPERTY(BlueprintGetter=GetShopPurchaseWeaponArray)
	TArray<UShopPurchaseWeaponWidget*> ShopPurchaseWeaponArray;

	UPROPERTY(EditDefaultsOnly, Category="UI")
	TSubclassOf<UShopPurchaseWeaponWidget> ShopPurchaseWeaponWidgetClass;

	UPROPERTY()
	UShopPurchaseWeaponWidget* SelectedShopPurchaseWidget;

	UPROPERTY(meta=(BindWidget))
	UScrollBox* PurchaseWeaponScroll;

	UPROPERTY(meta=(BindWidget))
	UImage* ImageWeapon;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeaponNameText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeaponWeightText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeaponSellPriceText;
	
	UPROPERTY(meta=(BindWidget))
	UVerticalBox* WeaponDataVerticalBox;

	UPROPERTY(meta=(BindWidget), BlueprintGetter=GetShopPerkImage)
	UShopPerkImageWidget* ShopPerkImage;

	UPROPERTY()
	TArray<UShopQuickPerkSelectWidget*> ShopQuickPerkSelectWidgetArray;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* RemainTimeText;
	UPROPERTY(meta=(BindWidget))
	UTextBlock* WaveText;
	UPROPERTY(meta=(BindWidget))
	UBasicButtonWidget* SellWeaponButton;
	UPROPERTY(meta=(BindWidget))
	UBasicButtonWidget* PurchaseWeaponButton;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* MoneyText;

	UPROPERTY()
	TArray<UImage*> WeightImageArray;

	UPROPERTY(meta=(BindWidget))
	UTextBlock* WeightImageText;

	void RefreshPerkQuickSelect(FGameplayTag Channel, const FGameplayMessage_Perk& Message);
	UFUNCTION()
	void RefreshPerkQuickSelect(EPerkType PerkType);
	void UpdateWaveText();
	UFUNCTION()
	void UpdateWeightImage(FGameplayTag Channel, const FGameplayMessage_None& Message);
	void UpdateWeightImage();
	void UpdateMoneyText(FGameplayTag Channel, const FGameplayMessage_PlayerMoneyChange& Message);
	void UpdateMoneyText(int32 Money);
	void BreakTimeChanged(FGameplayTag Channel, const FGameplayMessage_BreakEndTimeChanged& Message);
	FTimerHandle CountdownTimerHandle;
	void UpdateTimerText(int32 EndTime);

public:
	UFUNCTION(BlueprintGetter, BlueprintPure)
	TArray<UShopPlayerWeaponWidget*>& GetShopPlayerWeaponArray()
	{
		return ShopPlayerWeaponArray;
	}

	UFUNCTION(BlueprintGetter, BlueprintPure)
	class APlayerCharacterController* GetPlayerCharacterController() const
	{
		return PlayerCharacterController;
	}

	UFUNCTION(BlueprintGetter, BlueprintPure)
	class UShopPerkImageWidget* GetShopPerkImage() const
	{
		return ShopPerkImage;
	}

	UFUNCTION(BlueprintGetter, BlueprintPure)
	TArray<UShopPurchaseWeaponWidget*>& GetShopPurchaseWeaponArray()
	{
		return ShopPurchaseWeaponArray;
	}

	UFUNCTION(BlueprintCallable)
	void OnEnter();


	UFUNCTION(BlueprintCallable)
	void OnExit();

	UFUNCTION()
	void InitUIList();

	UFUNCTION(blueprintCallable)
	void SellWeapon();
	void RefreshPlayerWeapon(FGameplayTag Channel, const FGameplayMessage_None& Message);

	UFUNCTION(blueprintCallable)
	void RefreshPlayerWeapon();

	UFUNCTION(blueprintCallable)
	void PlayerWeaponBtnClick(UShopPlayerWeaponWidget* ClickedShopPlayerWeaponWidget);
	void UpdatePlayerWeaponImage();
	void RefreshPurchaseWeapon(FGameplayTag Channel, const FGameplayMessage_None& Message);

	UFUNCTION(blueprintCallable)
	void RefreshPurchaseWeapon();

	UFUNCTION(BlueprintCallable)
	void PurchaseWeaponBtnClickEvent(UShopPurchaseWeaponWidget* ClickedShopPurchaseWeaponWidget);

	void UpdatePurchaseWeaponImage();
	UFUNCTION(BlueprintCallable)
	void PurchaseWeapon();

	FGameplayMessageListenerHandle ListenerHandle;
};
