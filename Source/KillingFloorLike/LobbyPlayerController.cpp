// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyPlayerController.h"
#include "LobbyGameMode.h"

bool ALobbyPlayerController::Server_RequestStartGame_Validate(const FString& MapPath)
{
	// 맵 경로가 비어있지 않은지 등 간단한 유효성 검사를 할 수 있습니다.
	return !MapPath.IsEmpty();
}

void ALobbyPlayerController::Server_RequestStartGame_Implementation(const FString& MapPath)
{
	// 이 코드는 서버에서 실행됩니다.
	ALobbyGameMode* LobbyGameMode = GetWorld()->GetAuthGameMode<ALobbyGameMode>();
	if (LobbyGameMode)
	{
		// TODO: 이 요청을 보낸 플레이어가 방장(Host)인지 확인하는 권한 검사 로직이 필요합니다.
		// if (IsHostPlayer()) { ... }

		// GameMode의 함수를 호출하여 모든 플레이어를 이동시킵니다.
		LobbyGameMode->EnterKFGameMode(MapPath);
	}
}