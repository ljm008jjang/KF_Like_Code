// Fill out your copyright notice in the Description page of Project Settings.


#include "CrossHair.h"

#include "GameTextLibrary.h"
#include "KillingFloorLikeCharacter.h"
#include "KillingFloorLikeGameState.h"
#include "PerkIconWithStarWidget.h"
#include "PlayerCharacterController.h"
#include "PubSubManager.h"
#include "RangeWeapon.h"
#include "ShopManager.h"
#include "VomitDamageType.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/WidgetSwitcher.h"
#include "Kismet/GameplayStatics.h"

void UCrossHair::NativeConstruct()
{
	Super::NativeConstruct();
	
	OverallSwitcher->SetActiveWidget(MainCanvas);

	CacheActorPointers();
	SubscribeToEvents();

	// Initialize UI with current game state
	if (IsValid(GameState))
	{
		SwitchInfoDisplay(GameState->GetCurrentModeType());
		UpdateWaveText();
		StartBreakdownTimer(GameState->GetBreakEndTime());
	}

	if (IsValid(OwningPlayerCharacter))
	{
		UpdateHealthAndArmorText(OwningPlayerCharacter->GetCurrentHP(), OwningPlayerCharacter->GetCurrentArmor());
		UpdateMoneyText(OwningPlayerCharacter->GetMoney());
		UpdateWeightText();
		UpdateAmmoAndGrenadeText();
		PerkIconWithStar->SetPerk(OwningPlayerCharacter->GetPerkData());
	}
}

void UCrossHair::CacheActorPointers()
{
	GameState = Cast<AKillingFloorLikeGameState>(UGameplayStatics::GetGameState(GetWorld()));
	PlayerCharacterController = Cast<APlayerCharacterController>(GetOwningPlayer());
	OwningPlayerCharacter = Cast<AKillingFloorLikeCharacter>(GetOwningPlayerPawn());

	if (!IsValid(GameState)) UE_LOG(LogTemp, Warning, TEXT("UCrossHair::CacheActorPointers - GameState is not valid."));
	if (!IsValid(PlayerCharacterController)) UE_LOG(LogTemp, Warning, TEXT("UCrossHair::CacheActorPointers - PlayerCharacterController is not valid."));
	if (!IsValid(OwningPlayerCharacter)) UE_LOG(LogTemp, Warning, TEXT("UCrossHair::CacheActorPointers - OwningPlayerCharacter is not valid."));
}

void UCrossHair::SubscribeToEvents()
{
	UPubSubManager* PubSubManager = GetGameInstance()->GetSubsystem<UPubSubManager>();
	if (!IsValid(PubSubManager))
	{
		UE_LOG(LogTemp, Error, TEXT("UCrossHair::SubscribeToEvents - PubSubManager is not valid!"));
		return;
	}

	PubSubManager->Subscribe(EPubSubTag::Mode_End, this, &UCrossHair::OnEndMatch);
	PubSubManager->Subscribe(EPubSubTag::Mode_MonsterCountChange, this, &UCrossHair::OnMonsterCountChanged);
	PubSubManager->Subscribe(EPubSubTag::Mode_ModeTypeChanged, this, &UCrossHair::OnGameModeChanged);
	PubSubManager->Subscribe(EPubSubTag::Mode_BreakEndTimeChanged, this, &UCrossHair::OnBreakEndTimeChanged);
	
	PubSubManager->Subscribe(EPubSubTag::Player_AmmoChange, this, &UCrossHair::OnPlayerAmmoChanged);
	PubSubManager->Subscribe(EPubSubTag::Player_MoneyChange, this, &UCrossHair::OnPlayerMoneyChanged);
	PubSubManager->Subscribe(EPubSubTag::Player_WeaponChange, this, &UCrossHair::OnPlayerWeightChanged);
	PubSubManager->Subscribe(EPubSubTag::Player_HealthChange, this, &UCrossHair::OnPlayerHealthChanged);
}

void UCrossHair::NativeDestruct()
{
	GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	Super::NativeDestruct();
}


void UCrossHair::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	UpdateShopDistanceText();
}

void UCrossHair::OnEndMatch(FGameplayTag Channel, const FGameplayMessage_EndMatch& Message)
{
	if (Message.IsWin)
	{
		OverallSwitcher->SetActiveWidget(SurvivedImage);
	}else
	{
		OverallSwitcher->SetActiveWidget(WipedOutImage);
	}
}

void UCrossHair::OnMonsterCountChanged(FGameplayTag Channel, const FGameplayMessage_MonsterCount& Message)
{
	UpdateMonsterCountText(Message.RemainMonsterCount);
}

void UCrossHair::UpdateMonsterCountText(int32 RemainMonsterCount) const
{
	MonsterText->SetText(FText::AsNumber(RemainMonsterCount));
}

void UCrossHair::OnGameModeChanged(FGameplayTag Channel, const FGameplayMessage_GameMode& Message)
{
	SwitchInfoDisplay(Message.ModeType);
}

void UCrossHair::SwitchInfoDisplay(EModeType CurrentModeType)
{
	switch (CurrentModeType)
	{
	case EModeType::Wave:
		UpdateWaveText();
		InfoSwitcher->SetActiveWidgetIndex(0);
		break;
	case EModeType::Break:
	case EModeType::None:

		InfoSwitcher->SetActiveWidgetIndex(1);
		break;
	default:
		break;
	}
}

void UCrossHair::OnBreakEndTimeChanged(FGameplayTag Channel, const FGameplayMessage_BreakEndTimeChanged& Message)
{
	StartBreakdownTimer(Message.BreakEndTime);
}

void UCrossHair::StartBreakdownTimer(int32 BreakEndTime)
{
	GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, FTimerDelegate::CreateUObject(
											   this, &UCrossHair::UpdateBreakdownTimer, BreakEndTime), 0.1f, true);
}

void UCrossHair::UpdateBreakdownTimer(int32 EndTime)
{
	// 매초 "종료 시각"을 기준으로 남은 시간을 새로 계산합니다.
	const float TimeRemaining = FMath::Max(0.0f, EndTime - GetWorld()->GetTimeSeconds());
	const int32 TimeRemainingInt = FMath::FloorToInt(TimeRemaining);

	BreakTimeText->SetText(FText::AsNumber(TimeRemainingInt));

	// BreakTime 종료 5초 전에 애니메이션 재생
	if (TimeRemainingInt <= 5)
	{
		PlayWaveInboundAnimation();
	}


	if (TimeRemaining <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
}

void UCrossHair::UpdateWaveText() const
{
	if (!IsValid(GameState)) return;
	WaveText->SetText(GameState->GetWaveText());
}


void UCrossHair::Client_PlayHitAnimation(TSubclassOf<UDamageType> DamageType, float CurrentHp)
{
	if (DamageType == UVomitDamageType::StaticClass())
	{
		if (VomitAnimation)
		{
			PlayAnimation(VomitAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
		}
	}
	else
	{
		if (HitAnimation)
		{
			PlayAnimation(HitAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
		}
	}

	if (CurrentHp > 20.0f)
	{
		HitSerious->SetVisibility(ESlateVisibility::Hidden);
	}
	else
	{
		HitSerious->SetVisibility(ESlateVisibility::Visible);
	}
}

void UCrossHair::PlayWaveInboundAnimation()
{
	if (WaveInboundAnimation && IsAnimationPlaying(WaveInboundAnimation) == false)
	{
		PlayAnimation(WaveInboundAnimation, 0.0f, 1, EUMGSequencePlayMode::Forward, 1.0f);
	}
}

void UCrossHair::OnPlayerHealthChanged(FGameplayTag Channel, const FGameplayMessage_PlayerHeathChange& Message)
{
	if (Message.ListenerObject != GetOwningPlayerPawn())
	{
		return;
	}
	UpdateHealthAndArmorText(Message.Health, Message.Armor);
}

void UCrossHair::UpdateHealthAndArmorText(int32 Health, int32 Armor) const
{
	if (!IsValid(HpText) || !IsValid(ArmorText)) return;
	
	HpText->SetText(FText::AsNumber(Health));
	ArmorText->SetText(FText::AsNumber(Armor));
}

void UCrossHair::UpdateTradeTextVisibility(bool bIsVisible) const
{
	if (!IsValid(TradeText)) return;
	
	TradeText->SetVisibility(bIsVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UCrossHair::UpdateShopDistanceText() const
{
	if (!IsValid(ShopDistText)) return;
	if (!IsValid(GameState) || !IsValid(GameState->GetShopManager())) return;
	
	ShopDistText->SetText(GameState->GetShopManager()->GetShopDistText());
}

void UCrossHair::OnPlayerAmmoChanged(FGameplayTag Channel, const FGameplayMessage_PlayerAmmoChange& Message)
{
	if (Message.ListenerObject != GetOwningPlayerPawn())
	{
		return;
	}
	UpdateAmmoAndGrenadeText();
}

void UCrossHair::UpdateAmmoAndGrenadeText() const
{
	if (!IsValid(OwningPlayerCharacter) || !IsValid(BulletText) || !IsValid(ClipText) || !IsValid(GranadeText)) return;

	if (ABaseWeapon* CurrentWeapon = OwningPlayerCharacter->GetCurrentWeapon())
	{
		BulletText->SetText(CurrentWeapon->GetAmmoText());
		ClipText->SetText(CurrentWeapon->GetClipText());
	}
	else
	{
		BulletText->SetText(FText::FromString(""));
		ClipText->SetText(FText::FromString(""));
	}

	if (ABaseWeapon* GrenadeWeapon = OwningPlayerCharacter->GetWeapon(EWeaponType::Grenade))
	{
		GranadeText->SetText(GrenadeWeapon->GetAmmoText());
	}
	else
	{
		GranadeText->SetText(FText::AsNumber(0));
	}
}

void UCrossHair::OnPlayerMoneyChanged(FGameplayTag Channel, const FGameplayMessage_PlayerMoneyChange& Message)
{
	if (Message.ListenerObject != GetOwningPlayerPawn())
	{
		return;
	}

	UpdateMoneyText(Message.Money);
}

void UCrossHair::UpdateMoneyText(int32 Money) const
{
	if (!IsValid(MoneyText)) return;
	MoneyText->SetText(UGameTextLibrary::FormatMoney(Money));
}

void UCrossHair::OnPlayerWeightChanged(FGameplayTag Channel, const FGameplayMessage_None& Message)
{
	if (Message.ListenerObject != GetOwningPlayerPawn())
	{
		return;
	}
	UpdateWeightText();
}

void UCrossHair::UpdateWeightText() const
{
	if (!IsValid(WeightText)) return;
	if (!IsValid(OwningPlayerCharacter))
	{
		WeightText->SetText(FText::GetEmpty());
		return;
	}
	WeightText->SetText(OwningPlayerCharacter->GetWeightText());
}
