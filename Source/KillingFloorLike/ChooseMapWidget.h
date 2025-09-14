// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "KFLikeGameInstance.h"
#include "Blueprint/UserWidget.h"
#include "ChooseMapWidget.generated.h"

class UImage;
class UEditableText;
class UBasicButtonWidget;
class FOnlineSessionSettings;
class UWidgetSwitcher;
class UBasicCheckBoxWidget;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UChooseMapWidget : public UUserWidget
{
	GENERATED_BODY()
	UChooseMapWidget(const FObjectInitializer& ObjectInitializer);
	UChooseMapWidget(FVTableHelper& Helper);
	virtual bool Initialize() override;
	
	// TUniquePtr 멤버의 소멸자가 .cpp 파일에서 생성되도록 소멸자를 명시적으로 선언합니다.
	~UChooseMapWidget();
private:
	UPROPERTY(meta=(BindWidget))
	UBasicCheckBoxWidget* SelectMapCheckBox;
	UPROPERTY(meta=(BindWidget))
	UBasicCheckBoxWidget* SelectServerRuleCheckBox;
	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* HostSwitcher;
	UPROPERTY(meta=(BindWidget))
	UWidget* SelectMapPanel;
	UPROPERTY(meta=(BindWidget))
	UWidget* ServerRulePanel;
	UPROPERTY(meta=(BindWidget))
	UBasicButtonWidget* BackButton;
	UPROPERTY(meta=(BindWidget))
	UBasicButtonWidget* HostButton;
	UPROPERTY(meta=(BindWidget))
	UEditableText* ServerNameEditableText;
	UPROPERTY(meta=(BindWidget))
	UPanelWidget* MapList;
	UPROPERTY(meta=(BindWidget))
	UImage* MapImage;

	UPROPERTY()
	UBasicCheckBoxWidget* SelectedCheckBox;
	
	TUniquePtr<FOnlineSessionSettings> SessionSettings;


	TSubclassOf<UUserWidget> MapRowClass;

	TOptional<uint32> SelectedMapIndex;

	FString DesiredServerName;


	UFUNCTION()
	void OnSelectMapCheckBox(bool IsChecked);
	UFUNCTION()
	void OnSelectServerRuleCheckBox(bool IsChecked);
	UFUNCTION()
	void OnClickHost();
	UFUNCTION()
	void OnClickBack();
	void SetMapList();
	UFUNCTION()
	void OnServerNameChanged(const FText& Text);
public:
	virtual void SetUp();
	void SelectIndex(uint32 Index);
	const FMapData* GetMapData(int32 SelectedIndex);
	void UpdateChildren();
	void UpdateMapImage(uint32 Index);
};
