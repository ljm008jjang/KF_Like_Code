// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MainWidget.h"
#include "Blueprint/UserWidget.h"
#include "JoinServerWidget.generated.h"

class UBasicButtonWidget;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UJoinServerWidget : public UUserWidget
{
	GENERATED_BODY()
	UJoinServerWidget(const FObjectInitializer& ObjectInitializer);

	virtual bool Initialize() override;
	
private:
	UPROPERTY()
	UMainWidget* Parent;

	UPROPERTY(meta=(BindWidget))
	UBasicButtonWidget* RefreshButton;
	UPROPERTY(meta=(BindWidget))
	UBasicButtonWidget* JoinButton;
	UPROPERTY(meta=(BindWidget))
	UBasicButtonWidget* BackButton;

	UPROPERTY(meta=(BindWidget))
	UPanelWidget* ServerList;
	TSubclassOf<UUserWidget> ServerRowClass;

	TOptional<uint32> SelectedIndex;

	void UpdateChildren();
	
	UFUNCTION()
	void OnClickRefresh();
	UFUNCTION()
	void OnClickJoin();
	UFUNCTION()
	void OnClickBack();


public:
	void SetUp(class UMainWidget* NewParent);
	
	void SelectIndex(uint32 Index);
	void SetServerList(TArray<FServerData> Servers);

	void JoinServer();
};
