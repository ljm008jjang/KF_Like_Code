// Fill out your copyright notice in the Description page of Project Settings.


#include "JoinServerWidget.h"

#include "BasicButtonWidget.h"
#include "ServerRowWidget.h"
#include "Components/Button.h"
#include "Components/PanelWidget.h"
#include "Components/TextBlock.h"

UJoinServerWidget::UJoinServerWidget(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> ServerRowBPClass(TEXT("/Game/UI/WB_ServerRow"));
	if (ServerRowBPClass.Class == nullptr)
	{
		return;
	}
	ServerRowClass = ServerRowBPClass.Class;
}

bool UJoinServerWidget::Initialize()
{
	bool Result = Super::Initialize();
	
	RefreshButton->Button->OnClicked.AddDynamic(this, &UJoinServerWidget::OnClickRefresh);
	JoinButton->Button->OnClicked.AddDynamic(this, &UJoinServerWidget::OnClickJoin);
	BackButton->Button->OnClicked.AddDynamic(this, &UJoinServerWidget::OnClickBack);

	RefreshButton->SetText(FText::FromString("Refresh"));
	JoinButton->SetText(FText::FromString("Join"));
	BackButton->SetText(FText::FromString("Back"));

	return Result;
}

void UJoinServerWidget::SelectIndex(uint32 Index)
{
	SelectedIndex = Index;
	UpdateChildren();
}

void UJoinServerWidget::SetServerList(TArray<FServerData> Servers)
{
	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return;
	}

	ServerList->ClearChildren();

	uint32 i = 0;
	for (const FServerData& ServerData : Servers)
	{
		UServerRowWidget* Row = CreateWidget<UServerRowWidget>(World, ServerRowClass);
		if (Row == nullptr)
		{
			UE_LOG(LogTemp, Warning, TEXT("Row is null"));
			return;
		}

		Row->ServerNameText->SetText(FText::FromString(ServerData.ServerName));
		Row->MapNameText->SetText(FText::FromString(ServerData.MapName));
		Row->PlayersText->SetText(
			FText::FromString(FString::Printf(TEXT("%d/%d"), ServerData.CurrentPlayers, ServerData.MaxPlayers))
		);
		Row->WaveText->SetText(
			FText::FromString(FString::Printf(TEXT("%d/%d"), ServerData.CurrentWave, ServerData.MaxWave))
		);
		Row->Setup(this, i);
		++i;

		ServerList->AddChild(Row);
	}
}

void UJoinServerWidget::UpdateChildren()
{
	for (int32 i = 0; i < ServerList->GetChildrenCount(); ++i)

	{
		UServerRowWidget* Row = Cast<UServerRowWidget>(ServerList->GetChildAt(i));
		if (Row == nullptr)
		{
			continue;
		}

		Row->Selected = (SelectedIndex.IsSet() && i == SelectedIndex.GetValue());
	}
}

void UJoinServerWidget::SetUp(UMainWidget* NewParent)
{
	Parent = NewParent;
}

void UJoinServerWidget::OnClickRefresh()
{
	if (Parent)
	{
		Parent->OpenJoinMenu();
	}
}
 
void UJoinServerWidget::OnClickJoin()
{
	if (Parent)
	{
		Parent->JoinServer(SelectedIndex);
	}
}
 
void UJoinServerWidget::OnClickBack()
{
	if (Parent)
	{
		Parent->OnClickBack();
	}
}