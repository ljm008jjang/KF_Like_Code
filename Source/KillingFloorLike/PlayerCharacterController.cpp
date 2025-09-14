// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterController.h"

#include "CrossHair.h"
#include "KFLikeGameInstance.h"
#include "KillingFloorHud.h"
#include "KillingFloorLikeCharacter.h"

void APlayerCharacterController::BeginPlay()
{
	Super::BeginPlay();
	KillingFloorHud = Cast<AKillingFloorHud>(GetHUD());
	KFCharacter = Cast<AKillingFloorLikeCharacter>(GetCharacter());

	GetGameInstance()->GetSubsystem<UPubSubManager>()->Subscribe(EPubSubTag::Mode_End,this,
																 &APlayerCharacterController::ShowExitUI);
}

void APlayerCharacterController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(ExitUITimeHandle);
	Super::EndPlay(EndPlayReason);
}

void APlayerCharacterController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);

	// 마우스 커서가 보이지 않도록 설정합니다.
	bShowMouseCursor = false;
}

void APlayerCharacterController::ShowExitUI(FGameplayTag Channel, const FGameplayMessage_EndMatch& Message)
{
	UKFLikeGameInstance* GI = GetGameInstance<UKFLikeGameInstance>();
	GetWorld()->GetTimerManager().ClearTimer(ExitUITimeHandle);
	GetWorld()->GetTimerManager().SetTimer(ExitUITimeHandle, FTimerDelegate::CreateLambda( [GI]()
	{
		if (GI)
		{
			// 이제 안전하게 로컬 GameInstance의 함수를 호출하여 UI를 띄웁니다.
			GI->ShowExitMenuUI(true);
			
		}
	}), 3, false);
}

void APlayerCharacterController::ShowShopWidget_Implementation(bool IsVisible)
{
	GetKillingFloorHud()->GetWbCrossHairInstance()->
	                      UpdateTradeTextVisibility(IsVisible);
}
