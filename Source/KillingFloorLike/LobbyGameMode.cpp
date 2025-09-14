// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"

#include "KFLikeGameInstance.h"
#include "Kismet/KismetSystemLibrary.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		// 커맨드 라인에서 서버 이름 가져오기 (없으면 기본값 사용)
		FString ServerName = TEXT("KF-Like Dedicated Servder");
		FParse::Value(FCommandLine::Get(), TEXT("-ServerName="), ServerName);

		// 현재 맵 이름을 가져옵니다. 서버 실행 시 맵 이름이 지정됩니다.
		FString MapName = GetWorld()->GetMapName();
		MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

		UE_LOG(LogTemp, Log, TEXT("ALobbyGameMode Lobby Dedicated Server starting session with ServerName: %s on Map: %s"), *ServerName, *MapName);

		Cast<UKFLikeGameInstance>(GetGameInstance())->HostDedicatedServer(MapName, ServerName);
	}
}

void ALobbyGameMode::EnterKFGameMode(FString MapPath)
{
	//
	UWorld* WorldPtr = GetWorld();
	if (WorldPtr)
	{
		WorldPtr->ServerTravel(MapPath + "?listen");
	}
}
