// Fill out your copyright notice in the Description page of Project Settings.


#include "KFLikeGameInstance.h"

#include "ChooseMapWidget.h"
#include "JoinServerWidget.h"
#include "KillingFloorLikeGameMode.h"
#include "KillingFloorLikeGameState.h"
#include "MainWidget.h"
#include "OnlineSessionSettings.h"
#include "OnlineSubsystemUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


class FAssetRegistryModule;

UKFLikeGameInstance::UKFLikeGameInstance(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer)
{
	ConstructorHelpers::FClassFinder<UUserWidget> MenuBPClass(TEXT("/Game/UI/WB_Main"));
	if (IsValid(MenuBPClass.Class) == false)
	{
		return;
	}
	MenuClass = MenuBPClass.Class;

	ConstructorHelpers::FClassFinder<UChooseMapWidget> ChooseMapBPClass(TEXT("/Game/UI/WB_ChooseMap"));
	if (IsValid(ChooseMapBPClass.Class) == false)
	{
		return;
	}
	ChooseMapClass = ChooseMapBPClass.Class;

	ConstructorHelpers::FClassFinder<UUserWidget> ExitBPClass(TEXT("/Game/UI/WB_Exit"));
	if (IsValid(ExitBPClass.Class) == false)
	{
		return;
	}
	ExitClass = ExitBPClass.Class;
}

void UKFLikeGameInstance::Init()
{
	Super::Init();
	InitDataTable();
	UE_LOG(LogTemp, Warning, TEXT("GameInstance Init Called")); // 로그 추가

	IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
	if (Subsystem == nullptr)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Found subsystem %s"), *Subsystem->GetSubsystemName().ToString());
	SessionInterface = Subsystem->GetSessionInterface();
	if (SessionInterface.IsValid() == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("SessionInterface IsValid"));
		return;
	}

	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(
		this, &UKFLikeGameInstance::OnCreateSessionComplete);
	SessionInterface->OnDestroySessionCompleteDelegates.AddUObject(
		this, &UKFLikeGameInstance::OnDestroySessionComplete);
	SessionInterface->OnUpdateSessionCompleteDelegates.AddUObject(
		 this, &UKFLikeGameInstance::OnUpdateSessionComplete);
	SessionInterface->OnFindSessionsCompleteDelegates.AddUObject(
		this, &UKFLikeGameInstance::OnFindSessionComplete);
	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(
		this, &UKFLikeGameInstance::OnJoinSessionComplete);

	GEngine->OnNetworkFailure().AddUObject(this, &UKFLikeGameInstance::OnNetworkFailure);


	UE_LOG(LogTemp, Display, TEXT("Found session interface"));


	/*// 이 프로그램이 데디케이트 서버로 실행되었을 때만 세션을 호스팅합니다.
	if (UKismetSystemLibrary::IsDedicatedServer(GetWorld()))
	{
		// 커맨드 라인에서 서버 이름 가져오기 (없으면 기본값 사용)
		FString ServerName = TEXT("KF-Like Dedicated Server");
		FParse::Value(FCommandLine::Get(), TEXT("-ServerName="), ServerName);

		// 현재 맵 이름을 가져옵니다. 서버 실행 시 맵 이름이 지정됩니다.
		FString MapName = GetWorld()->GetMapName();
		MapName.RemoveFromStart(GetWorld()->StreamingLevelsPrefix);

		UE_LOG(LogTemp, Log, TEXT("Lobby Dedicated Server starting session with ServerName: %s on Map: %s"), *ServerName, *MapName);

		HostDedicatedServer(MapName, ServerName);
	}*/
}

TArray<FWeaponData> UKFLikeGameInstance::GetWeaponDataArray()
{
	if (WeaponDataArray.IsEmpty())
	{
		for (auto const& Result : WeaponDataMap)
		{
			WeaponDataArray.Add(Result.Value);
		}
	}

	return WeaponDataArray;
}

TSubclassOf<ABaseWeapon> UKFLikeGameInstance::GetWeaponClass(int32 id)
{
	if (BaseWeaponClassMap.Contains(id) == false)
	{
		UE_LOG(LogTemp, Display, TEXT("weapon id = %d"), id);
		return nullptr;
	}
	return BaseWeaponClassMap[id];
}

TSubclassOf<AActor> UKFLikeGameInstance::GetGeneralObjectClass(EGenetalObjectType ObjectType) const
{
	// FindRef는 키가 없을 경우 nullptr을 안전하게 반환합니다.
	return GeneralObjectClassMap.FindRef(ObjectType);
}

const FWeaponData* UKFLikeGameInstance::GetWeaponData(int32 id) const
{
	// Find는 키가 없을 경우 nullptr을 안전하게 반환합니다.
	return WeaponDataMap.Find(id);
}

const FMonsterData* UKFLikeGameInstance::GetMonsterData(int32 id) const
{
	return MonsterDataMap.Find(id);
}

const FPerkData* UKFLikeGameInstance::GetPerkData(const EPerkType PerkType, const int32 Rank) const
{
	if (const FPerkLevelMap* LevelData = PerkDataMap.Find(PerkType))
	{
		return LevelData->LevelToDataMap.Find(Rank);
	}
	return nullptr;
}

const FPerkConstData* UKFLikeGameInstance::GetPerkConstData(const EPerkType PerkType) const
{
	return PerkConstDataMap.Find(PerkType);
}

FString UKFLikeGameInstance::GetPerkImagePath(const EPerkType PerkType, const int32 Rank) const
{
	const FPerkConstData* PerkConstData = GetPerkConstData(PerkType);
	if (!PerkConstData)
	{
		return FString();
	}

	return (Rank >= 6) ? PerkConstData->gold_image_path : PerkConstData->image_path;
}

FString UKFLikeGameInstance::GetWeaponImagePath(const int32 ID) const
{
	const FWeaponData* data = GetWeaponData(ID);
	if (data)
	{
		return data->image_path;
	}
	return FString();
}

FString UKFLikeGameInstance::GetPlayerBlendSpacePath(const int32 WeaponID, bool IsSit) const
{
	const FWeaponData* data = GetWeaponData(WeaponID);
	if (data)
	{
		const FString Suffix = IsSit ? TEXT("_CH") : TEXT("");
		const FString AssetName = FString::Printf(TEXT("BS_%s%s"), *data->animation_name, *Suffix);
		return FString::Printf(TEXT("%s/%s.%s"), *PlayerBlendSpacePath, *AssetName, *AssetName);
	}
	return FString();
}

FString UKFLikeGameInstance::GetPlayerAnimMontagePath(const int32 WeaponID, EWeaponAnimationType WeaponAnimation) const
{
	const FWeaponData* data = GetWeaponData(WeaponID);
	if (!data)
	{
		return FString();
	}

	FString AdditionalPath = "";
	if (WeaponAnimation == EWeaponAnimationType::Fire)
	{
		AdditionalPath = PlayerAnimMontagePath + "/" + "Attack";
	}

	// 기존 로직은 접두사, 애니메이션 이름, 경로를 합치고 있었습니다.
	// 기능 변경 없이 가독성만 개선합니다.
	return FString::Printf(TEXT("BS_%s%s"), *data->animation_name, *AdditionalPath);
}

const TMap<int32, FMapData>* UKFLikeGameInstance::GetMapDataMap() const
{
	return &MapDataMap;
}

const FMapData* UKFLikeGameInstance::GetMapData(int32 Id) const
{
	return MapDataMap.Find(Id);
}

const TArray<FMapData>* UKFLikeGameInstance::GetMapDataArray() const
{
	return &MapDataArray;
}

void UKFLikeGameInstance::ShowExitMenuUI(bool IsHard)
{
	if (IsValid(ExitClass) == false)
	{
		return;
	}
	if (IsValid(ExitWidget) == false)
	{
		ExitWidget = CreateWidget<UUserWidget>(this, ExitClass);
		if (IsValid(ExitWidget) == false)
		{
			return;
		}
	}

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	// 하드 모드에서는 항상 메뉴를 표시하고 게임을 일시정지합니다.
	if (IsHard)
	{
		if (!ExitWidget->IsInViewport())
		{
			ExitWidget->AddToViewport();
		}
		PlayerController->SetPause(true);
		PlayerController->SetShowMouseCursor(true);
		return;
	}

	// 일반 모드에서는 메뉴 표시 상태를 토글합니다.
	const bool bShouldBeVisible = !ExitWidget->IsInViewport();
	if (bShouldBeVisible)
	{
		ExitWidget->AddToViewport();
	}
	else
	{
		ExitWidget->RemoveFromParent();
	}

	PlayerController->SetPause(bShouldBeVisible);
	PlayerController->SetShowMouseCursor(bShouldBeVisible);
}

void UKFLikeGameInstance::InitDataTable()
{
	LoadWeaponDataTables();
	LoadGeneralObjectDataTable();
	LoadMonsterDataTable();
	LoadPerkDataTables();
	LoadMapDataTable();
}

bool UKFLikeGameInstance::LoadDataTable (const FString& Path, UDataTable*& OutDataTable)
{
	UE_LOG(LogTemp, Display, TEXT("Start Data : %s"), *Path);
	UDataTable* LoadedTable = Cast<UDataTable>(StaticLoadObject(UDataTable::StaticClass(), nullptr, *Path));
	if (LoadedTable)
	{
		UE_LOG(LogTemp, Display, TEXT("Success Data: %s"), *Path);
		OutDataTable = LoadedTable;
		return true;
	}
	UE_LOG(LogTemp, Warning, TEXT("Failed to load data table: %s"), *Path);
	return false;
};

void UKFLikeGameInstance::LoadWeaponDataTables()
{
	UDataTable* NewWeaponDataTable = nullptr;
	if (LoadDataTable(TEXT("DataTable'/Game/Data/_weapon._weapon'"), NewWeaponDataTable))
	{
		for (const auto& RowPair : NewWeaponDataTable->GetRowMap())
		{
			if (const FWeaponData* Row = reinterpret_cast<FWeaponData*>(RowPair.Value))
			{
				// FindOrAdd는 키가 없으면 새로 추가하고, 있으면 기존 값을 반환합니다.
				// 이를 통해 중복 데이터에 대한 안정성을 높입니다.
				WeaponDataMap.FindOrAdd(Row->id) = *Row;
			}
		}
		// 데이터 로딩 후 클래스 정보 설정
		SetBaseWeaponClasses(NewWeaponDataTable);
	}
}

void UKFLikeGameInstance::LoadGeneralObjectDataTable()
{
	UDataTable* NewGeneralObjectDataTable = nullptr;
	if (LoadDataTable(TEXT("DataTable'/Game/Data/_general_object._general_object'"), NewGeneralObjectDataTable))
	{
		SetGeneralObjectClasses(NewGeneralObjectDataTable);
	}
}

void UKFLikeGameInstance::LoadMonsterDataTable()
{
	UDataTable* NewMonsterDataTable = nullptr;
	LoadDataTable(TEXT("DataTable'/Game/Data/_monster._monster'"), NewMonsterDataTable);
	if (NewMonsterDataTable)
	{
		SetMonsterClasses(NewMonsterDataTable);
	}
}

void UKFLikeGameInstance::LoadPerkDataTables()
{
	UDataTable* NewPerkDataTable = nullptr;
	if (LoadDataTable(TEXT("DataTable'/Game/Data/perk.perk'"), NewPerkDataTable))
	{
		for (const auto& RowPair : NewPerkDataTable->GetRowMap())
		{
			if (const FPerkData* Row = reinterpret_cast<FPerkData*>(RowPair.Value))
			{
				FPerkLevelMap& LevelData = PerkDataMap.FindOrAdd(Row->type);
				LevelData.LevelToDataMap.FindOrAdd(Row->rank) = *Row;
			}
		}
	}

	UDataTable* NewPerkConstDataTable = nullptr;
	if (LoadDataTable(TEXT("DataTable'/Game/Data/_perk_const._perk_const'"), NewPerkConstDataTable))
	{
		for (const auto& RowPair : NewPerkConstDataTable->GetRowMap())
		{
			if (const FPerkConstData* Row = reinterpret_cast<FPerkConstData*>(RowPair.Value))
			{
				PerkConstDataMap.FindOrAdd(Row->type) = *Row;
			}
		}
	}
}

void UKFLikeGameInstance::LoadMapDataTable()
{
	UDataTable* NewMapDataTable = nullptr;
	if (LoadDataTable(TEXT("DataTable'/Game/Data/_map._map'"), NewMapDataTable))
	{
		MapDataArray.Empty();
		for (const auto& RowPair : NewMapDataTable->GetRowMap())
		{
			if (const FMapData* Row = reinterpret_cast<FMapData*>(RowPair.Value))
			{
				MapDataMap.FindOrAdd(Row->id) = *Row;
				MapDataArray.Add(*Row);
			}
		}
	}
}

void UKFLikeGameInstance::SetBaseWeaponClasses(const UDataTable* NewWeaponDataTable)
{
	BaseWeaponClassMap.Empty();

	// 애셋 레지스트리 모듈 가져오기
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");

	for (const auto& Row : NewWeaponDataTable->GetRowMap())
	{
		const FName RowName = Row.Key;
		const int32 NameAsInt = FCString::Atoi(*RowName.ToString());
		const FWeaponData* WeaponData = reinterpret_cast<FWeaponData*>(Row.Value);
		const FString AssetName = "BP_" + (WeaponData->name.Replace(TEXT(" "), TEXT("")));
		const FString AssetPath = FString::Printf(TEXT("/Game/BluePrint/Weapon/%s.%s_C"), *AssetName, *AssetName);

		TSubclassOf<ABaseWeapon> LoadedClass = Cast<UClass>(
			StaticLoadObject(UClass::StaticClass(), nullptr, *AssetPath));

		if (LoadedClass)
		{
			// 참고: CDO(Class Default Object)를 수정하는 것은 잠재적인 부작용을 일으킬 수 있습니다.
			// 각 무기 인스턴스가 생성될 때 데이터를 설정하는 것이 더 안전한 설계입니다.
			LoadedClass->GetDefaultObject<ABaseWeapon>()->SetId(NameAsInt);
			LoadedClass->GetDefaultObject<ABaseWeapon>()->SetWeaponData(*WeaponData);
			BaseWeaponClassMap.Add(NameAsInt, LoadedClass); // 배열에 추가
		}
	}
}

void UKFLikeGameInstance::SetGeneralObjectClasses(const UDataTable* NewGeneralObjectDataTable)
{
	GeneralObjectClassMap.Empty();

	for (const auto& Row : NewGeneralObjectDataTable->GetRowMap())
	{
		const FGeneralObjectData* GeneralObjectData = reinterpret_cast<FGeneralObjectData*>(Row.Value);
		const FString AssetPath = GeneralObjectData->path + "_C";
		UE_LOG(LogTemp, Display, TEXT("GeneralObjectDataTable path : %s"), *AssetPath);

		TSubclassOf<AActor> LoadedClass = Cast<UClass>(
			StaticLoadObject(UClass::StaticClass(), nullptr, *AssetPath));

		if (LoadedClass)
		{
			GeneralObjectClassMap.Emplace(GeneralObjectData->type, LoadedClass);
		}
	}

	for (const auto& Pair : GeneralObjectClassMap)
	{
		UE_LOG(LogTemp, Display, TEXT("Loaded General Object: %s"), *Pair.Value->GetName());
	}
}

void UKFLikeGameInstance::SetMonsterClasses(const UDataTable* NewMonsterDataTable)
{
	MonsterDataMap.Empty();
	// 애셋 레지스트리 모듈 가져오기
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<
		FAssetRegistryModule>("AssetRegistry");

	TArray<FMonsterData*> AllRows;
	NewMonsterDataTable->GetAllRows(TEXT(""), AllRows);

	for (const auto& Row : NewMonsterDataTable->GetRowMap())
	{
		const FMonsterData* Data = reinterpret_cast<FMonsterData*>(Row.Value);
		const EMonsterType Type = Data->type;
		const FString StringType = StaticEnum<EMonsterType>()->GetNameStringByValue(static_cast<int64>(Type));
		const FString AssetName = "BP_" + StringType;
		// 참고: 에셋 이름에 공백이 포함될 경우를 대비하여 Replace를 유지하는 것이 좋습니다.
		const FString AssetPath = FString::Printf(TEXT("/Game/BluePrint/Character/%s.%s_C"), *AssetName, *AssetName);

		TSubclassOf<AMonster> LoadedClass = Cast<UClass>(
			StaticLoadObject(UClass::StaticClass(), nullptr, *AssetPath));

		if (LoadedClass)
		{
			LoadedClass->GetDefaultObject<AMonster>()->SetMonsterType(Type);
			LoadedClass->GetDefaultObject<AMonster>()->SetMonsterData(*Data);

			MonsterDataMap.Emplace(Data->id,*Data);
		}
	}
}

void UKFLikeGameInstance::LoadMenuWidget()
{
	if (IsValid(MenuClass) == false)
	{
		return;
	}
	MainWidget = CreateWidget<UMainWidget>(this, MenuClass);
	if (IsValid(MainWidget) == false)
	{
		return;
	}

	MainWidget->SetUp();

	MainWidget->SetMenuInterface(this);
}

void UKFLikeGameInstance::Host(FOnlineSessionSettings* SessionSettings)
{
	if (SessionSettings == nullptr || SessionInterface.IsValid() == false)
	{
		return;
	}
	OnlineSessionSettings = MakeShareable(new FOnlineSessionSettings(*SessionSettings));

	if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Destroy Session"));
		SessionInterface->DestroySession(NAME_GameSession);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Host Session"));
		CreateSession();
	}
}

void UKFLikeGameInstance::LoadChooseMapWidget()
{
	if (IsValid(ChooseMapClass) == false)
	{
		return;
	}
	ChooseMapWidget = CreateWidget<UChooseMapWidget>(this, ChooseMapClass);
	if (IsValid(ChooseMapWidget) == false)
	{
		return;
	}
	ChooseMapWidget->SetUp();
	//ChooseMapWidget->SetMenuInterface(this);
}

void UKFLikeGameInstance::HostDedicatedServer(const FString& MapName, const FString& ServerName)
{
	if (SessionInterface.IsValid() == false)
	{
		return;
	}

	// 데디케이트 서버용 세션 설정
	TSharedPtr<FOnlineSessionSettings> DedicatedSessionSettings = MakeShareable(new FOnlineSessionSettings());
	DedicatedSessionSettings->NumPublicConnections = 6; // 최대 플레이어 수
	DedicatedSessionSettings->bIsLANMatch = false; // LAN이 아닌 인터넷 매치
	DedicatedSessionSettings->bAllowInvites = true;
	DedicatedSessionSettings->bIsDedicated = true;
	DedicatedSessionSettings->bUsesPresence = false;
	DedicatedSessionSettings->bUseLobbiesIfAvailable = false;
	DedicatedSessionSettings->bAllowJoinInProgress = true;
	DedicatedSessionSettings->bAllowJoinViaPresence = false;
	DedicatedSessionSettings->bShouldAdvertise = false; // 다른 플레이어가 찾을 수 있도록 광고
	DedicatedSessionSettings->Set(SERVER_NAME_SETTINGS_KEY, ServerName,
	                              EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	DedicatedSessionSettings->Set(MAP_INDEX_SETTINGS_KEY, 0,
	                              EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	DedicatedSessionSettings->Set(CURRENT_WAVE_SETTINGS_KEY, 0, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	DedicatedSessionSettings->Set(MAX_WAVE_SETTINGS_KEY, 10, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
	OnlineSessionSettings = DedicatedSessionSettings;

	if (SessionInterface->GetNamedSession(NAME_GameSession) != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Destory dedicated Session"));
		SessionInterface->DestroySession(NAME_GameSession);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Host dedicated Session"));
		CreateSession();
	}
}

void UKFLikeGameInstance::Join(uint32 Index)
{
	if (SessionInterface.IsValid() == false)
	{
		return;
	}

	if (SessionSearch.IsValid() == false)
	{
		return;
	}

	if (IsValid(MainWidget))
	{
		MainWidget->Teardown();
	}

	SessionInterface->JoinSession(0, NAME_GameSession, SessionSearch->SearchResults[Index]);
}

void UKFLikeGameInstance::StartSession()
{
	if (SessionInterface.IsValid() == false)
	{
		return;
	}

	SessionInterface->StartSession(NAME_GameSession);
}

void UKFLikeGameInstance::LoadMainMenu()
{
	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	PlayerController->ClientTravel("/Game/Map/Start_Level", TRAVEL_Absolute);
}


void UKFLikeGameInstance::RefreshServerList()
{
	SessionSearch = MakeShareable(new FOnlineSessionSearch());
	if (SessionSearch.IsValid())
	{
		SessionSearch->MaxSearchResults = 100;
		SessionSearch->QuerySettings.Set(FName(TEXT("PRESENCESEARCH")), true, EOnlineComparisonOp::Equals);
		UE_LOG(LogTemp, Display, TEXT("Starting find Session"));
		SessionInterface->FindSessions(0, SessionSearch.ToSharedRef());
	}
}

void UKFLikeGameInstance::OnCreateSessionComplete(FName SessionName, bool Success)
{
	if (Success == false)
	{
		UE_LOG(LogTemp, Display, TEXT("Couldn't create session : %s"), *SessionName.ToString());
		return;
	}


	UEngine* Engine = GetEngine();
	if (IsValid(Engine) == false)
	{
		return;
	}

	Engine->AddOnScreenDebugMessage(0, 2, FColor::Green,TEXT("Hosting"));

	// 리슨 서버인 경우에만 즉시 ServerTravel을 실행합니다.
	// 데디케이트 서버는 세션 생성 후 대기 상태로 들어갑니다.
	if (OnlineSessionSettings.IsValid() && OnlineSessionSettings->bIsDedicated)
	{
		// 데디케이트 서버는 세션 생성 후 아무것도 하지 않고 로비에서 대기합니다.
		UE_LOG(LogTemp, Log, TEXT("Dedicated server session created successfully. Waiting for players in lobby."));
	}
	else
	{
		int32 MapIndex;
		OnlineSessionSettings->Get(MAP_INDEX_SETTINGS_KEY, MapIndex);
		const FMapData* MapData = GetMapData(MapIndex);
		if (MapData)
		{
			UWorld* WorldPtr = GetWorld();
			if (WorldPtr)
			{
				WorldPtr->ServerTravel(MapData->path + "?listen");
			}
		}
	}
}

void UKFLikeGameInstance::OnDestroySessionComplete(FName SessionName, bool Success)
{
	// IsHosting 플래그 대신, 이 코드가 서버에서 실행되는지 클라이언트에서 실행되는지 직접 확인하는 것이 훨씬 더 안정적입니다.
	// GetWorld()->IsServer()는 현재 월드가 서버(리슨 서버 포함)인지 확인합니다.
	if (GetWorld() && GetWorld()->GetNetMode() == NM_DedicatedServer)
	{
		// 서버가 세션을 파괴한 경우 (예: 새 맵으로 호스팅하기 위해)
		// 다시 세션을 생성하여 다른 플레이어들이 접속할 수 있도록 준비합니다.
		if (Success)
		{
			UE_LOG(LogTemp, Display, TEXT("Server successfully destroyed session. Recreating... : %s"), *SessionName.ToString());
			CreateSession();
		}
	}
	else
	{
		// 클라이언트가 세션을 떠난 경우, 메인 메뉴로 안전하게 이동합니다.
		LoadMainMenu();
	}
}
void UKFLikeGameInstance::OnUpdateSessionComplete(FName SessionName, bool Success)
{
	if (Success)
	{
		UE_LOG(LogTemp, Log, TEXT("Session '%s' updated successfully."), *SessionName.ToString());
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Failed to update session '%s'."), *SessionName.ToString());
	}
}
void UKFLikeGameInstance::OnFindSessionComplete(bool Success)
{
	if (Success == false || SessionSearch.IsValid() == false || IsValid(MainWidget) == false)
	{
		return;
	}

	TArray<FServerData> ServerNames;
	for (const FOnlineSessionSearchResult& SearchResult : SessionSearch->SearchResults)
	{
		UE_LOG(LogTemp, Display, TEXT("Found session name : %s"), *SearchResult.GetSessionIdStr());
		FServerData Data;
		Data.MaxPlayers = SearchResult.Session.SessionSettings.NumPublicConnections;
		Data.CurrentPlayers = Data.MaxPlayers - SearchResult.Session.NumOpenPublicConnections;

		// 세션 설정에서 웨이브 정보를 읽어옵니다.
		int32 WaveNumber = 0; // 기본값
		SearchResult.Session.SessionSettings.Get(CURRENT_WAVE_SETTINGS_KEY, WaveNumber);
		Data.CurrentWave = WaveNumber;
		int32 MaxWaveNumber = 10; // 기본값
		SearchResult.Session.SessionSettings.Get(MAX_WAVE_SETTINGS_KEY, MaxWaveNumber);
		Data.MaxWave = MaxWaveNumber;

		int32 MapIndex = 0;
		if (SearchResult.Session.SessionSettings.Get(MAP_INDEX_SETTINGS_KEY, MapIndex))
		{
			if (const FMapData* MapData = GetMapData(MapIndex))
			{
				Data.MapName = MapData->name;
			}
		}
		else
		{
			Data.MapName = "Unknown Map";
		}
		
		SearchResult.Session.SessionSettings.Get(SERVER_NAME_SETTINGS_KEY, Data.ServerName);
		ServerNames.Add(Data);
	}
	MainWidget->GetJoinServerWidget()->SetServerList(ServerNames);

	UE_LOG(LogTemp, Display, TEXT("Finish find Session"));
}

void UKFLikeGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (SessionInterface.IsValid() == false)
	{
		return;
	}

	FString Address;
	if (SessionInterface->GetResolvedConnectString(SessionName, Address) == false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Couldn't get connect string"));
		return;
	}
	UEngine* Engine = GetEngine();
	if (IsValid(Engine) == false)
	{
		return;
	}

	Engine->AddOnScreenDebugMessage(0, 5, FColor::Green, FString::Printf(TEXT("Joining %s"), *Address));

	APlayerController* PlayerController = GetFirstLocalPlayerController();
	if (IsValid(PlayerController) == false)
	{
		return;
	}

	PlayerController->ClientTravel(Address, TRAVEL_Absolute);
}

void UKFLikeGameInstance::OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type Arg,
                                           const FString& String)
{
	// 서버와의 연결이 끊어졌을 때 호출됩니다.
	// 메인 메뉴로 돌아가기 전에, 현재 참여 중인 세션이 있다면 파괴하여 상태를 정리해야 합니다.
	if (SessionInterface.IsValid())
	{
		// GetNamedSession을 사용하여 현재 세션이 유효한지 확인하고, 그렇다면 파괴합니다.
		// OnDestroySessionComplete 델리게이트가 LoadMainMenu를 호출해 줄 것입니다.
		SessionInterface->DestroySession(NAME_GameSession);
	}
	else
	{
		// 세션 시스템을 사용하지 않는 경우(오프라인 등) 즉시 메인 메뉴로 이동합니다.
		LoadMainMenu();
	}
}

void UKFLikeGameInstance::CreateSession()
{
	if (SessionInterface.IsValid() == false)
	{
		return;
	}
	if (OnlineSessionSettings.IsValid() == false)
	{
		return;
	}

	// --- 세션 설정 정보 출력 시작 ---
	UE_LOG(LogTemp, Log, TEXT("--- OnlineSessionSettings Info ---"));
	UE_LOG(LogTemp, Log, TEXT("Num Public Connections: %d"), OnlineSessionSettings->NumPublicConnections);
	UE_LOG(LogTemp, Log, TEXT("Num Private Connections: %d"), OnlineSessionSettings->NumPrivateConnections);
	UE_LOG(LogTemp, Log, TEXT("Is LAN Match: %s"), OnlineSessionSettings->bIsLANMatch ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Log, TEXT("Should Advertise: %s"),
	       OnlineSessionSettings->bShouldAdvertise ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Log, TEXT("Uses Presence: %s"),
	       OnlineSessionSettings->bUsesPresence ? TEXT("true") : TEXT("false"));
	UE_LOG(LogTemp, Log, TEXT("Allow Join In Progress: %s"),
	       OnlineSessionSettings->bAllowJoinInProgress ? TEXT("true") : TEXT("false"));

	// 커스텀 설정(Settings Map) 출력
	UE_LOG(LogTemp, Log, TEXT("Custom Settings:"));
	for (const auto& Setting : OnlineSessionSettings->Settings)
	{
		// Setting.Key는 FName, Setting.Value는 FOnlineSessionSetting 구조체입니다.
		const FName Key = Setting.Key;
		const FOnlineSessionSetting& Value = Setting.Value;
		UE_LOG(LogTemp, Log, TEXT("  - %s = %s"), *Key.ToString(), *Value.ToString());
	}
	UE_LOG(LogTemp, Log, TEXT("------------------------------------"));

	UE_LOG(LogTemp, Warning, TEXT("Start CreateSession"));
	bool bStarted = SessionInterface->CreateSession(0, NAME_GameSession, *OnlineSessionSettings);
	if (!bStarted)
	{
		UE_LOG(LogTemp, Error, TEXT("CreateSession call failed immediately!"));
	}
}

void UKFLikeGameInstance::CheckGameModeAndExit(APlayerController* PlayerController) // 함수 이름은 원하는 대로 정할 수 있습니다.
{
	UWorld* World = GetWorld();
	if (IsValid(World) == false || IsValid(PlayerController) == false)
	{
		return;
	}

	// 클라이언트에서는 GameMode에 접근할 수 없으므로, 모든 클라이언트에게 복제되는 GameState로 확인해야 합니다.
	AGameStateBase* CurrentGameState = UGameplayStatics::GetGameState(World);

	// 현재 게임 상태가 AKillingFloorLikeGameState 클래스인지 확인합니다.
	if (Cast<AKillingFloorLikeGameState>(CurrentGameState))
	{
		// 게임 플레이 중이므로, 메인 메뉴 맵으로 이동합니다.
		UE_LOG(LogTemp, Log, TEXT("Currently in a game level. Returning to the main menu."));

		if (PlayerController)
		{
			// ClientTravel 함수는 서버, 클라이언트, 싱글플레이어 모든 경우를 알아서 처리해주는 가장 확실한 방법입니다.
			if (SessionInterface.IsValid())
			{
				SessionInterface->DestroySession(NAME_GameSession);
				// 참고: DestroySession은 비동기로 작동하지만, ClientTravel을 즉시 호출해도 대부분의 경우 문제가 없습니다.
				// 만약 문제가 발생한다면, OnDestroySessionComplete 델리게이트를 바인딩하여
				// 세션 파괴가 완료된 후 ClientTravel을 호출하도록 변경할 수 있습니다.
			}
			else
			{
				// 세션 인터페이스가 없는 경우 (오프라인 등) 바로 이동합니다.
				PlayerController->ClientTravel(TEXT("/Game/Map/Start_Level"), ETravelType::TRAVEL_Absolute);
			}
		}
	}
	else
	{
		// 게임 플레이 중이 아니므로 (예: 메인 메뉴), 게임을 종료합니다.
		UE_LOG(LogTemp, Log, TEXT("Not in a game level. Quitting game."));

		UKismetSystemLibrary::QuitGame(World, PlayerController, EQuitPreference::Quit, true);
	}
}
