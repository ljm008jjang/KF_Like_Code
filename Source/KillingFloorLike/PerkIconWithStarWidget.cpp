// Fill out your copyright notice in the Description page of Project Settings.


#include "PerkIconWithStarWidget.h"

#include "KFLikeGameInstance.h"
#include "PubSubManager.h"
#include "ResourceManager.h"
#include "Components/Image.h"

void UPerkIconWithStarWidget::NativeConstruct()
{
	Super::NativeConstruct();

	GetGameInstance()->GetSubsystem<UPubSubManager>()->Subscribe(EPubSubTag::Player_PerkChange, this,
	                                                             &UPerkIconWithStarWidget::SetPerk);

	StarImageArray.Empty();
	for (int32 i = 1; i < 6; i++)
	{
		FString WidgetName = FString::Printf(TEXT("Image_star_%d"), i);
		if (UWidget* FoundWidget = GetWidgetFromName(FName(*WidgetName)))
		{
			if (UImage* FoundImage = Cast<UImage>(FoundWidget))
			{
				StarImageArray.Add(FoundImage);
			}
		}
	}
}

void UPerkIconWithStarWidget::NativeDestruct()
{
	/*if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (UPubSubManager* PubSubManager = GameInstance->GetSubsystem<UPubSubManager>())
		{
			// 이 위젯(this)에 등록된 모든 구독을 해지합니다.
			PubSubManager->Unsubscribe(this);
		}
	}*/
	
	Super::NativeDestruct();
}

void UPerkIconWithStarWidget::SetPerk(FGameplayTag Channel, const FGameplayMessage_Perk& Message)
{
	if (Message.ListenerObject != GetOwningPlayerPawn())
	{
		return;
	}
	const FPerkData& PerkData = Message.PerkData;
	SetPerk(PerkData);
}

void UPerkIconWithStarWidget::SetPerk(const FPerkData& PerkData)
{
	UKFLikeGameInstance* KFLikeGameInstance = Cast<UKFLikeGameInstance>(GetGameInstance());
	if (IsValid(KFLikeGameInstance) == false)
	{
		return;
	}
	UTexture2D* Texture = GetGameInstance()->GetSubsystem<UResourceManager>()->LoadTexture(
		KFLikeGameInstance->GetPerkImagePath(PerkData.type, PerkData.rank));
	PerkImage->SetBrushFromTexture(Texture);

	int32 index = 0;
	for (UImage* StarImage : StarImageArray)
	{
		if (PerkData.rank == 6)
		{
			StarImage->SetVisibility(ESlateVisibility::Hidden);
		}
		else
		{
			if (PerkData.rank > index)
			{
				StarImage->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				StarImage->SetVisibility(ESlateVisibility::Hidden);
			}
		}
		index++;
	}
}
