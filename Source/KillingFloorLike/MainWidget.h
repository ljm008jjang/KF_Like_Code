// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainWidget.generated.h"

class FOnlineSessionSettings;
class UJoinServerWidget;
class UHostServerWidget;
class UEditableTextBox;
class IMenuInterface;
class UKFButton;
class UBasicButtonWidget;
class UWidgetSwitcher;
class UButton;

USTRUCT()
struct FServerData
{
	GENERATED_BODY()

	FString ServerName;
	FString MapName;
	uint16 CurrentPlayers;
	uint16 MaxPlayers;
	uint16 CurrentWave;
	uint16 MaxWave;
};

UCLASS()
class KILLINGFLOORLIKE_API UMainWidget : public UUserWidget
{
	GENERATED_BODY()
	UMainWidget(const FObjectInitializer& ObjectInitializer);

private:
	UPROPERTY(meta=(BindWidget))
	UWidgetSwitcher* MainSwitcher;
	UPROPERTY(meta=(BindWidget))
	UHostServerWidget* HostServerWidget;
	UPROPERTY(meta=(BindWidget))
	UJoinServerWidget* JoinServerWidget;


	UPROPERTY(meta=(BindWidget))
	UKFButton* HostServerButton;
	UPROPERTY(meta=(BindWidget))
	UKFButton* JoinServerButton;


	UFUNCTION()
	void OnClickHostServer();

protected:
	IMenuInterface* MenuInterface;

	virtual bool Initialize() override;


public:
	UHostServerWidget* GetHostServerWidget() const
	{
		return HostServerWidget;
	}

	UJoinServerWidget* GetJoinServerWidget() const
	{
		return JoinServerWidget;
	}

	UFUNCTION()
	void OnClickBack();
	UFUNCTION()
	void OpenHostMenu();
	UFUNCTION()
	void OpenJoinMenu();
	void HostServer(FOnlineSessionSettings* SessionSettings);
	UFUNCTION()
	void JoinServer(TOptional<uint32> SelectedIndex);

	void SetMenuInterface(IMenuInterface* NewMenuInterface);
	virtual void SetUp();
	virtual void Teardown();
};
