// Fill out your copyright notice in the Description page of Project Settings.


#include "MapRowWidget.h"

#include "ChooseMapWidget.h"
#include "HostServerWidget.h"
#include "KFCheckBox.h"
#include "Components/TextBlock.h"

void UMapRowWidget::Setup(UUserWidget* NewParent, uint32 NewIndex)
{
	Parent = NewParent;
	Index = NewIndex;
	RowCheckBox->OnCheckStateChanged.AddDynamic(this, &UMapRowWidget::OnStateChanged);
}

void UMapRowWidget::OnStateChanged(bool IsCheck)
{
	if (IsCheck == false)
	{
		return;
	}
	if (UChooseMapWidget* ChooseMapWidget = Cast<UChooseMapWidget>(Parent))
	{
		ChooseMapWidget->SelectIndex(Index);
	}else if (UHostServerWidget* HostServerWidget = Cast<UHostServerWidget>(Parent))
	{
		HostServerWidget->SelectIndex(Index);
	}
}

void UMapRowWidget::UpdateVisuals()
{
	// 이 함수는 부모 위젯에 의해 직접 호출되어,
	// 데이터 상태('Selected' 변수)와 시각적 상태(폰트 크기)를 일치시킵니다.
	FSlateFontInfo InFontInfo = MapNameText->GetFont();
	InFontInfo.Size = Selected ? 20 : 16;
	MapNameText->SetFont(InFontInfo);
}
