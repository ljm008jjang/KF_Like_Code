// Fill out your copyright notice in the Description page of Project Settings.


#include "MainWidget.h"

#include "BasicButtonWidget.h"
#include "HostServerWidget.h"
#include "JoinServerWidget.h"
#include "KFButton.h"
#include "MenuInterface.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/WidgetSwitcher.h"


UMainWidget::UMainWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
}

void UMainWidget::OnClickHostServer()
{
	MainSwitcher->SetActiveWidget(HostServerWidget);
}

void UMainWidget::OnClickBack()
{
	MainSwitcher->SetActiveWidgetIndex(0);
}

bool UMainWidget::Initialize()
{
	bool Success = Super::Initialize();
	HostServerButton->OnClicked.AddDynamic(this, &UMainWidget::OpenHostMenu);
	JoinServerButton->OnClicked.AddDynamic(this, &UMainWidget::OpenJoinMenu);
	HostServerWidget->SetUp(this);
	JoinServerWidget->SetUp(this);
	/*RefreshButton->Button->OnClicked.AddDynamic(this, &UMainWidget::OpenJoinMenu);
	JoinButton->Button->OnClicked.AddDynamic(this, &UMainWidget::JoinServer);
	BackButton->Button->OnClicked.AddDynamic(this, &UMainWidget::OnClickBack);

	RefreshButton->SetText(FText::FromString("Refresh"));
	JoinButton->SetText(FText::FromString("Join"));
	BackButton->SetText(FText::FromString("Back"));*/

	return Success;
}

void UMainWidget::HostServer(FOnlineSessionSettings* SessionSettings)
{
	if (MenuInterface == nullptr)
	{
		return;
	}

	MenuInterface->Host(SessionSettings);
}

void UMainWidget::JoinServer(TOptional<uint32> SelectedIndex)
{
	if (SelectedIndex.IsSet() && MenuInterface != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index : %d"), SelectedIndex.GetValue());
		MenuInterface->Join(SelectedIndex.GetValue());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Selected index not set"));
	}
}

void UMainWidget::OpenHostMenu()
{
	MainSwitcher->SetActiveWidget(HostServerWidget);
}

void UMainWidget::OpenJoinMenu()
{
	MainSwitcher->SetActiveWidget(JoinServerWidget);
	if (MenuInterface != nullptr)
	{
		MenuInterface->RefreshServerList();
	}
}


void UMainWidget::SetMenuInterface(IMenuInterface* NewMenuInterface)
{
	this->MenuInterface = NewMenuInterface;
}

void UMainWidget::SetUp()
{
	AddToViewport();

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}


	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	FInputModeUIOnly InputModeData;
	InputModeData.SetWidgetToFocus(TakeWidget());
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = true;
}

void UMainWidget::Teardown()
{
	RemoveFromParent();

	UWorld* World = GetWorld();
	if (IsValid(World) == false)
	{
		return;
	}
	APlayerController* PlayerController = World->GetFirstPlayerController();
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	FInputModeGameOnly InputModeData;
	PlayerController->SetInputMode(InputModeData);
	PlayerController->bShowMouseCursor = false;
}
