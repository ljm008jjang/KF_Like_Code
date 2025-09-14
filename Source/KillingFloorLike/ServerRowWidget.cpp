// Fill out your copyright notice in the Description page of Project Settings.


#include "ServerRowWidget.h"

#include "JoinServerWidget.h"
#include "MainWidget.h"
#include "Components/Button.h"

void UServerRowWidget::Setup(UJoinServerWidget* NewParent, uint32 NewIndex)
{
	Parent = NewParent;
	Index = NewIndex;
	RowButton->OnClicked.AddDynamic(this, &UServerRowWidget::OnClicked);
}

void UServerRowWidget::OnClicked()
{
	Parent->SelectIndex(Index);
}