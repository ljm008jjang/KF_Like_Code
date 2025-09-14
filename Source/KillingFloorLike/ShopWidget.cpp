// Fill out your copyright notice in the Description page of Project Settings.


#include "ShopWidget.h"

#include "KFLikeGameInstance.h"
#include "KillingFloorLikeCharacter.h"
#include "KillingFloorLikeGameState.h"
#include "PlayerCharacterController.h"
#include "PubSubManager.h"
#include "ResourceManager.h"
#include "BasicButtonWidget.h"
#include "GameTextLibrary.h"
#include "ShopArmorWidget.h"
#include "ShopPerkImage.h"
#include "ShopPlayerWeaponWidget.h"
#include "Components/CheckBox.h"
#include "Components/ScrollBox.h"
#include "ShopPurchaseWeaponWidget.h"  // UShopPurchaseWeaponWidget 선언 헤더
#include "ShopQuickPerkSelectWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Kismet/GameplayStatics.h"


void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	PlayerCharacterController = Cast<APlayerCharacterController>(GetOwningPlayer());
	UPubSubManager* PubSubManager = GetGameInstance()->GetSubsystem<UPubSubManager>();
	PubSubManager->Subscribe(EPubSubTag::Player_PerkChange, this, &UShopWidget::RefreshPerkQuickSelect);
	PubSubManager->Subscribe(EPubSubTag::Player_WeaponChange, this, &UShopWidget::RefreshPlayerWeapon);
	PubSubManager->Subscribe(EPubSubTag::Player_WeaponChange, this, &UShopWidget::RefreshPurchaseWeapon);
	PubSubManager->Subscribe(EPubSubTag::Player_WeaponChange, this, &UShopWidget::UpdateWeightImage);
	PubSubManager->Subscribe(EPubSubTag::Player_MoneyChange, this, &UShopWidget::UpdateMoneyText);

	/*PubSubManager->Subscribe(EPubSubTag::Mode_BreakEndTimeChanged, this,
	                         &UShopWidget::BreakTimeChanged);*/
	GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, FTimerDelegate::CreateUObject(
		                                       this, &UShopWidget::UpdateTimerText,
		                                       (int32)Cast<AKillingFloorLikeGameState>(GetWorld()->GetGameState())->
		                                       GetBreakEndTime()), 0.1f, true);

	InitUIList();
}

void UShopWidget::NativeDestruct()
{
	/*if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPubSubManager* PubSubManager = GameInstance->GetSubsystem<UPubSubManager>())
		{
			// 이 위젯(this)에 등록된 모든 구독을 해지합니다.
			PubSubManager->Unsubscribe(this);
		}
	}*/
	GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	Super::NativeDestruct();
}

bool UShopWidget::Initialize()
{
	bool Result = Super::Initialize();
	SellWeaponButton->SetText(FText::FromString(TEXT("Sell")));
	PurchaseWeaponButton->SetText(FText::FromString(TEXT("Purchase")));

	KnifeWeaponWidget->SetButtonsNotVisible();

	return Result;
}

void UShopWidget::InitUIList()
{
	int32 Index = 0;
	ShopQuickPerkSelectWidgetArray.Empty();
	for (EPerkType WeaponType : TEnumRange<EPerkType>())
	{
		if (WeaponType == EPerkType::None)
		{
			continue;
		}
		FString WidgetName = FString::Printf(TEXT("WB_quickPerkSelectBtn_%d"), Index);
		if (UWidget* FoundWidget = GetWidgetFromName(FName(*WidgetName)))
		{
			if (UShopQuickPerkSelectWidget* FoundPerkSelectWidget = Cast<UShopQuickPerkSelectWidget>(FoundWidget))
			{
				ShopQuickPerkSelectWidgetArray.Add(FoundPerkSelectWidget);
				FoundPerkSelectWidget->Init(WeaponType, GetPlayerCharacterController()->GetKfCharacter());
			}
		}

		Index++;
	}
	WeightImageArray.Empty();
	for (int32 i = 0; i < 15; i++)
	{
		FString WidgetName = FString::Printf(TEXT("WeightImage_%d"), i);
		if (UWidget* FoundWidget = GetWidgetFromName(FName(*WidgetName)))
		{
			if (UImage* FoundImage = Cast<UImage>(FoundWidget))
			{
				WeightImageArray.Add(FoundImage);
			}
		}
	}
}

void UShopWidget::OnEnter()
{
	GetPlayerCharacterController()->SetShowMouseCursor(true);
	RefreshPlayerWeapon();
	RefreshPurchaseWeapon();

	RefreshPerkQuickSelect(GetPlayerCharacterController()->GetKfCharacter()->GetPerkType());
	UpdateWaveText();
	UpdateWeightImage();
	UpdateMoneyText(GetPlayerCharacterController()->GetKfCharacter()->GetMoney());

	WeaponDataVerticalBox->SetVisibility(ESlateVisibility::Collapsed);
}

void UShopWidget::RefreshPerkQuickSelect(FGameplayTag Channel, const FGameplayMessage_Perk& Message)
{
	if (Message.ListenerObject.Get() != GetOwningPlayer()->GetPawn())
	{
		return;
	}
	RefreshPerkQuickSelect(Message.PerkData.type);
}

void UShopWidget::RefreshPerkQuickSelect(EPerkType PlayerPerkType)
{
	ShopPerkImage->RefreshShopPerkImage(PlayerPerkType);

	for (UShopQuickPerkSelectWidget* Widget : ShopQuickPerkSelectWidgetArray)
	{
		if (Widget->GetPerkType() == PlayerPerkType)
		{
			Widget->SetVisibility(ESlateVisibility::Collapsed);
		}
		else
		{
			Widget->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void UShopWidget::UpdateWaveText()
{
	AKillingFloorLikeGameState* GameState = Cast<
		AKillingFloorLikeGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (GameState == nullptr)
	{
		return;
	}

	WaveText->SetText(GameState->GetWaveText());
}

void UShopWidget::UpdateWeightImage(FGameplayTag Channel, const FGameplayMessage_None& Message)
{
	if (Message.ListenerObject.Get() != GetOwningPlayer()->GetPawn())
	{
		return;
	}
	UpdateWeightImage();
}

void UShopWidget::UpdateWeightImage()
{
	int32 Index = 0;
	int32 Weight = GetPlayerCharacterController()->GetKfCharacter()->GetWeight();
	for (auto WeightImage : WeightImageArray)
	{
		if (Index >= Weight)
		{
			WeightImage->SetColorAndOpacity(FLinearColor::Black);
		}
		else
		{
			WeightImage->SetColorAndOpacity(FLinearColor::White);
		}
		Index++;
	}
	WeightImageText->SetText(FText::Format(
		FText::FromString(TEXT("Encumberance Level: {0}")),
		FText::AsNumber(Weight)
	));
}

void UShopWidget::UpdateMoneyText(FGameplayTag Channel, const FGameplayMessage_PlayerMoneyChange& Message)
{
	if (Message.ListenerObject.Get() != GetOwningPlayer()->GetPawn())
	{
		return;
	}

	UpdateMoneyText(Message.Money);
}

void UShopWidget::UpdateMoneyText(int32 Money)
{
	MoneyText->SetText(UGameTextLibrary::FormatMoney(Money));
}

void UShopWidget::BreakTimeChanged(FGameplayTag Channel, const FGameplayMessage_BreakEndTimeChanged& Message)
{
	GetWorld()->GetTimerManager().SetTimer(CountdownTimerHandle, FTimerDelegate::CreateUObject(
		                                       this, &UShopWidget::UpdateTimerText, Message.BreakEndTime), 0.1f, true);
}

void UShopWidget::UpdateTimerText(int32 EndTime)
{
	// 매초 "종료 시각"을 기준으로 남은 시간을 새로 계산합니다.
	const float TimeRemaining = FMath::Max(0.0f, EndTime - GetWorld()->GetTimeSeconds());
	const int32 TimeRemainingInt = FMath::FloorToInt(TimeRemaining);

	// 1. 남은 총 초를 '분'과 '초'로 변환합니다.
	const int32 Minutes = TimeRemainingInt / 60;
	const int32 Seconds = TimeRemainingInt % 60;

	// 2. FString::Printf를 사용해 "mm:ss" 형식의 문자열을 만듭니다.
	// 💡 %02d는 정수를 최소 2자리로 표현하고, 비어있는 앞자리는 0으로 채우라는 의미입니다.
	const FString TimeString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);

	// 3. 최종적으로 표시할 FText를 생성하고 위젯에 설정합니다.
	const FText TimerText = FText::Format(
		NSLOCTEXT("YourNamespace", "TraderClosesInFormat", "Trader Closes in {0}"),
		FText::FromString(TimeString)
	);
	RemainTimeText->SetText(TimerText);

	if (TimeRemaining <= 0.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	}
}


void UShopWidget::OnExit()
{
	if (IsInViewport() == false)
	{
		return;
	}

	GetPlayerCharacterController()->SetShowMouseCursor(false);
	GetPlayerCharacterController()->GetKfCharacter()->Server_ChangeUnitState(EUnitState::Idle);
	RemoveFromParent();
}


void UShopWidget::SellWeapon()
{
	if (SelectedShopPlayerWidget == nullptr || SelectedShopPlayerWidget->GetWeapon() == nullptr)
	{
		return;
	}

	GetPlayerCharacterController()->GetKfCharacter()->Server_SellWeapon(
		SelectedShopPlayerWidget->GetWeapon()->GetWeaponType());
}

void UShopWidget::RefreshPlayerWeapon(FGameplayTag Channel, const FGameplayMessage_None& Message)
{
	if (Message.ListenerObject.Get() != GetOwningPlayer()->GetPawn())
	{
		return;
	}
	RefreshPlayerWeapon();
}

void UShopWidget::RefreshPlayerWeapon()
{
	TArray<ABaseWeapon*> WeaponArray = GetPlayerCharacterController()->GetKfCharacter()->GetWeaponArray();

	int32 index = 0;
	for (auto BaseWeapon : WeaponArray)
	{
		if ((BaseWeapon->GetWeaponType() != EWeaponType::Main && BaseWeapon->GetWeaponType() != EWeaponType::Sub) &&
			BaseWeapon->GetWeaponData().can_sell == false)
		{
			continue;
		}
		//첫 생성
		if (GetShopPlayerWeaponArray().IsValidIndex(index) == false)
		{
			UShopPlayerWeaponWidget* NewShopPlayerWeaponWidget = CreateWidget<UShopPlayerWeaponWidget>(
				this, ShopPlayerWeaponWidgetClass);
			NewShopPlayerWeaponWidget->Refresh(BaseWeapon, this);
			GetShopPlayerWeaponArray().Add(NewShopPlayerWeaponWidget);
			PlayerWeaponScroll->AddChild(NewShopPlayerWeaponWidget);
		}
		else
		{
			GetShopPlayerWeaponArray()[index]->Refresh(BaseWeapon);
		}

		index++;
	}
	//오버되면 안보이게 처리
	for (int i = index; i < GetShopPlayerWeaponArray().Num(); i++)
	{
		GetShopPlayerWeaponArray()[i]->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SelectedShopPlayerWidget != nullptr)
	{
		SelectedShopPlayerWidget->WeaponCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		SelectedShopPlayerWidget = nullptr;
	}

	ABaseWeapon* Knife = GetPlayerCharacterController()->GetKfCharacter()->GetWeapon(EWeaponType::Knife);
	if (Knife != nullptr)
	{
		KnifeWeaponWidget->Refresh(Knife, this);
	}

	ABaseWeapon* Grenade = GetPlayerCharacterController()->GetKfCharacter()->GetWeapon(EWeaponType::Grenade);
	if (Grenade != nullptr)
	{
		GrenadesWeaponWidget->Refresh(Grenade, this);
	}
	

	CombatArmorWidget->Refresh(this);
}

void UShopWidget::PlayerWeaponBtnClick(UShopPlayerWeaponWidget* ClickedShopPlayerWeaponWidget)
{
	if (ClickedShopPlayerWeaponWidget == nullptr)
	{
		return;
	}

	if (SelectedShopPlayerWidget == ClickedShopPlayerWeaponWidget)
	{
		SelectedShopPlayerWidget = nullptr;
	}
	else
	{
		if (SelectedShopPlayerWidget != nullptr)
		{
			SelectedShopPlayerWidget->WeaponCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}
		SelectedShopPlayerWidget = ClickedShopPlayerWeaponWidget;
	}

	UpdatePlayerWeaponImage();
}

void UShopWidget::UpdatePlayerWeaponImage()
{
	if (SelectedShopPlayerWidget == nullptr)
	{
		return;
	}
	UResourceManager* ResourceManager = GetGameInstance()->GetSubsystem<UResourceManager>();
	
	if (ResourceManager == nullptr)
	{
		return;
	}
	const FWeaponData& WeaponData = SelectedShopPlayerWidget->GetWeapon()->GetWeaponData();

	UTexture2D* ImageTexture = ResourceManager->LoadWeaponImageTexture(
		WeaponData.id);

	if (ImageTexture == nullptr)
	{
		return;
	}

	ImageWeapon->SetBrushFromTexture(ImageTexture);
	WeaponNameText->SetText(FText::FromString(WeaponData.name));
	WeaponWeightText->SetText(
		FText::FromString(
			FString::Printf(TEXT("Weight: %d blocks"), SelectedShopPlayerWidget->GetWeapon()->GetWeaponData().weight)));
	if (WeaponData.can_sell)
	{
		WeaponSellPriceText->SetText(FText::FromString(
			FString::Printf(TEXT("Sell Value: $%d"), SelectedShopPlayerWidget->GetWeapon()->GetSellCost())));
		WeaponSellPriceText->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		WeaponSellPriceText->SetVisibility(ESlateVisibility::Hidden);
	}
	WeaponDataVerticalBox->SetVisibility(ESlateVisibility::Visible);
}

void UShopWidget::RefreshPurchaseWeapon(FGameplayTag Channel, const FGameplayMessage_None& Message)
{
	if (Message.ListenerObject.Get() != GetOwningPlayer()->GetPawn())
	{
		return;
	}
	RefreshPurchaseWeapon();
}

void UShopWidget::RefreshPurchaseWeapon()
{
	UKFLikeGameInstance* KFGameInstance = Cast<UKFLikeGameInstance>(GetGameInstance());
	if (KFGameInstance == nullptr)
	{
		return;
	}

	int32 Index = 0;
	for (FWeaponData& WeaponData : KFGameInstance->GetWeaponDataArray())
	{
		//플레이어한테 있거나 판매 불가
		if (GetPlayerCharacterController()->GetKfCharacter()->GetWeaponDataArray().Contains(WeaponData) ||
			WeaponData.can_sell == false)
		{
			continue;
		}

		if (GetShopPurchaseWeaponArray().IsValidIndex(Index) == false)
		{
			UShopPurchaseWeaponWidget* NewShopPurchaseWeaponWidget = CreateWidget<UShopPurchaseWeaponWidget>(
				this, ShopPurchaseWeaponWidgetClass);
			NewShopPurchaseWeaponWidget->Init(WeaponData, this);
			GetShopPurchaseWeaponArray().Add(NewShopPurchaseWeaponWidget);
			PurchaseWeaponScroll->AddChild(NewShopPurchaseWeaponWidget);
		}
		else
		{
			GetShopPurchaseWeaponArray()[Index]->Init(WeaponData, this);
		}


		Index++;
	}

	//오버되면 안보이게 처리
	for (int i = Index; i < GetShopPurchaseWeaponArray().Num(); i++)
	{
		GetShopPurchaseWeaponArray()[i]->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (SelectedShopPurchaseWidget != nullptr)
	{
		SelectedShopPurchaseWidget->WeaponCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		SelectedShopPurchaseWidget = nullptr;
	}
}

void UShopWidget::PurchaseWeaponBtnClickEvent(UShopPurchaseWeaponWidget* ClickedShopPurchaseWeaponWidget)
{
	if (ClickedShopPurchaseWeaponWidget == nullptr)
	{
		return;
	}

	if (SelectedShopPurchaseWidget == ClickedShopPurchaseWeaponWidget)
	{
		SelectedShopPurchaseWidget = nullptr;
	}
	else
	{
		if (SelectedShopPurchaseWidget != nullptr)
		{
			SelectedShopPurchaseWidget->WeaponCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}
		SelectedShopPurchaseWidget = ClickedShopPurchaseWeaponWidget;
	}

	UpdatePurchaseWeaponImage();
}

void UShopWidget::UpdatePurchaseWeaponImage()
{
	if (SelectedShopPurchaseWidget == nullptr)
	{
		return;
	}
	UResourceManager* ResourceManager = GetGameInstance()->GetSubsystem<UResourceManager>();
	
	if (ResourceManager == nullptr)
	{
		return;
	}

	UTexture2D* ImageTexture = ResourceManager->LoadWeaponImageTexture(
		SelectedShopPurchaseWidget->GetWeaponData().id);

	if (ImageTexture == nullptr)
	{
		return;
	}

	ImageWeapon->SetBrushFromTexture(ImageTexture);
	WeaponNameText->SetText(FText::FromString(SelectedShopPurchaseWidget->GetWeaponData().name));
	WeaponWeightText->SetText(
		FText::FromString(
			FString::Printf(TEXT("Weight: %d blocks"), SelectedShopPurchaseWidget->GetWeaponData().weight)));
	WeaponSellPriceText->SetVisibility(ESlateVisibility::Hidden);
	WeaponDataVerticalBox->SetVisibility(ESlateVisibility::Visible);
}

void UShopWidget::PurchaseWeapon()
{
	if (SelectedShopPurchaseWidget == nullptr)
	{
		return;
	}

	GetPlayerCharacterController()->GetKfCharacter()->Server_PurchaseWeapon(
		SelectedShopPurchaseWidget->GetWeaponData().id);
}
