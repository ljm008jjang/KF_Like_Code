// Fill out your copyright notice in the Description page of Project Settings.


#include "KFLikeGameViewportClient.h"

#include "KFLikeGameInstance.h"

bool UKFLikeGameViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	if (EventArgs.Event == IE_Pressed && EventArgs.Key == EKeys::Escape)
	{
		UE_LOG(LogTemp, Warning, TEXT("ESC 눌림!"));

		// 예시: 종료 UI 띄우기
		// 또는 GameInstance 호출해서 처리
		if (UKFLikeGameInstance* GI = Cast<UKFLikeGameInstance>(GetWorld()->GetGameInstance()))
		{
			UE_LOG(LogTemp, Warning, TEXT("Exit UI!"));
			GI->ShowExitMenuUI();
		}

		return true;
	}

	return Super::InputKey(EventArgs);
}
