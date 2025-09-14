// Fill out your copyright notice in the Description page of Project Settings.


#include "ChooseMapWidget.h"

#include "BasicButtonWidget.h"
#include "BasicCheckBoxWidget.h"
#include "KFCheckBox.h"
#include "KFLikeGameInstance.h"
#include "LobbyPlayerController.h"
#include "MainWidget.h"
#include "MapRowWidget.h"
#include "OnlineSessionSettings.h"
#include "ResourceManager.h"
#include "Components/Button.h"
#include "Components/CheckBox.h"
#include "Components/EditableText.h"
#include "Components/Image.h"
#include "Components/WidgetSwitcher.h"

UChooseMapWidget::UChooseMapWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> MapRowBPClass(TEXT("/Game/UI/WB_MapRow"));
	if (MapRowBPClass.Class == nullptr)
	{
		return;
	}
	MapRowClass = MapRowBPClass.Class;
}

UChooseMapWidget::UChooseMapWidget(FVTableHelper& Helper)
	: Super(Helper)
{
}

bool UChooseMapWidget::Initialize()
{
	bool Result = Super::Initialize();
	SelectMapCheckBox->SetText(FText::FromString("Select Map"));
	SelectServerRuleCheckBox->SetText(FText::FromString("Server Rules"));
	HostButton->SetText(FText::FromString("Host"));
	BackButton->SetText(FText::FromString("Back"));

	SelectMapCheckBox->CheckBox->OnCheckStateChanged.AddDynamic(this, &UChooseMapWidget::OnSelectMapCheckBox);
	SelectServerRuleCheckBox->CheckBox->OnCheckStateChanged.AddDynamic(
		this, &UChooseMapWidget::OnSelectServerRuleCheckBox);
	HostButton->Button->OnClicked.AddDynamic(this, &UChooseMapWidget::OnClickHost);
	BackButton->Button->OnClicked.AddDynamic(this, &UChooseMapWidget::OnClickBack);

	SelectMapCheckBox->CheckBox->SetCheckedState(ECheckBoxState::Checked);
	ServerNameEditableText->OnTextChanged.AddDynamic(this, &UChooseMapWidget::OnServerNameChanged);
	DesiredServerName = FString("Killing Floor");
	ServerNameEditableText->SetText(FText::FromString(DesiredServerName));

	SetMapList();
	return Result;
}

UChooseMapWidget::~UChooseMapWidget() = default;

void UChooseMapWidget::OnSelectMapCheckBox(bool IsChecked)
{
	if (IsChecked)
	{
		HostSwitcher->SetActiveWidget(SelectMapPanel);
		SelectServerRuleCheckBox->CheckBox->SetCheckedState(ECheckBoxState::Unchecked);
	}
	else
	{
		// 사용자가 이미 체크된 박스를 다시 클릭하여 체크를 해제하려고 할 때,
		// 다른 박스도 체크되지 않은 상태라면(즉, 아무것도 선택되지 않는 상태가 되려 한다면)
		// 다시 체크 상태로 되돌려 항상 하나는 선택되어 있도록 강제합니다.
		if (SelectServerRuleCheckBox->CheckBox->GetCheckedState() == ECheckBoxState::Unchecked)
		{
			SelectMapCheckBox->CheckBox->SetCheckedState(ECheckBoxState::Checked);
		}
	}
}

void UChooseMapWidget::OnSelectServerRuleCheckBox(bool IsChecked)
{
	if (IsChecked)
	{
		HostSwitcher->SetActiveWidget(ServerRulePanel);
		SelectMapCheckBox->CheckBox->SetCheckedState(ECheckBoxState::Unchecked);
	}
	else
	{
		if (SelectMapCheckBox->CheckBox->GetCheckedState() == ECheckBoxState::Unchecked)
		{
			SelectServerRuleCheckBox->CheckBox->SetCheckedState(ECheckBoxState::Checked);
		}
	}
}


void UChooseMapWidget::OnClickHost()
{

	if (SelectedMapIndex.IsSet() == false)
	{
		return;
	}
	
	const FMapData* MapData = GetMapData(SelectedMapIndex.GetValue());
	if (MapData == nullptr)
	{
		return;
	}
	
	Cast<ALobbyPlayerController>(GetOwningPlayer())->Server_RequestStartGame(MapData->path);
}

void UChooseMapWidget::OnClickBack()
{

}

void UChooseMapWidget::SetMapList()
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	// GameInstance를 캐스팅하고 null인지 확인하여 안정성을 높입니다.
	UKFLikeGameInstance* GameInstance = Cast<UKFLikeGameInstance>(GetGameInstance());
	if (GameInstance == nullptr)
	{
		return;
	}

	// 맵 데이터 배열 포인터를 가져와 null인지 확인합니다.
	const TArray<FMapData>* MapDataArray = GameInstance->GetMapDataArray();
	if (MapDataArray == nullptr)
	{
		return;
	}

	MapList->ClearChildren();

	uint32 i = 0;
	for (const FMapData& ServerData : *MapDataArray)
	{
		UMapRowWidget* Row = CreateWidget<UMapRowWidget>(World, MapRowClass);
		if (Row == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Failed to create MapRowWidget."));
			continue; // 하나의 행이 실패하더라도 계속 진행합니다.
		}

		Row->MapNameText->SetText(FText::FromString(ServerData.name));

		Row->Setup(this, i);
		++i;

		MapList->AddChild(Row);
	}
}

void UChooseMapWidget::OnServerNameChanged(const FText& Text)
{
	DesiredServerName = Text.ToString();
}

void UChooseMapWidget::SetUp()
{
	AddToViewport();

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}


	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (PlayerController == nullptr)
	{
		return;
	}

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = true;
}

void UChooseMapWidget::SelectIndex(uint32 Index)
{
	SelectedMapIndex = Index;
	UpdateChildren();
	UpdateMapImage(SelectedMapIndex.GetValue());
}

const FMapData* UChooseMapWidget::GetMapData(int32 SelectedIndex)
{
	//SelectedIndex는 0부터, mapData는 1부터
	UKFLikeGameInstance* GameInstance = Cast<UKFLikeGameInstance>(GetGameInstance());
	if (GameInstance == nullptr)
	{
		return nullptr;
	}
	return GameInstance->GetMapData(SelectedIndex + 1);
}

void UChooseMapWidget::UpdateChildren()
{
	for (int32 i = 0; i < MapList->GetChildrenCount(); ++i)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateChildren"));
		UMapRowWidget* Row = Cast<UMapRowWidget>(MapList->GetChildAt(i));
		if (Row == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Error"));
			continue;
		}

		Row->Selected = (SelectedMapIndex.IsSet() && i == SelectedMapIndex.GetValue());
		if (Row->Selected)
		{
			Row->RowCheckBox->SetCheckedState(ECheckBoxState::Checked);
		}
		else
		{
			Row->RowCheckBox->SetCheckedState(ECheckBoxState::Unchecked);
		}

		// 3. 자식의 시각적 상태를 데이터에 맞게 수동으로 업데이트합니다.
		Row->UpdateVisuals();
	}
}

void UChooseMapWidget::UpdateMapImage(uint32 Index)
{
	const FMapData* MapData = GetMapData(Index);
	if (MapData == nullptr)
	{
		// 맵 데이터가 없으면 이미지를 비웁니다.
		MapImage->SetBrush(FSlateBrush());
		return;
	}

	FString ImagePath = MapData->image_path;
	UTexture2D* MapTexture = GetGameInstance()->GetSubsystem<UResourceManager>()->LoadTexture(ImagePath);
	if (MapTexture)
	{
		MapImage->SetBrushFromTexture(MapTexture);
	}
	else
	{
		MapImage->SetBrush(FSlateBrush());
	}
}