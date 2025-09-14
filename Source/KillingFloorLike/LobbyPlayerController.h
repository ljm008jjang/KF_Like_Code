// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LobbyPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API ALobbyPlayerController : public ANoPawnPlayerController
{
	GENERATED_BODY()
public:
	/** 서버에게 게임 시작을 요청합니다. 클라이언트의 UI에서 호출됩니다. */
	UFUNCTION(Server, Reliable, WithValidation)
	void Server_RequestStartGame(const FString& MapPath);
};
