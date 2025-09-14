// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "BaseWeapon.h"
#include "KillingFloorLikeCharacter.h"
#include "MenuInterface.h"
#include "Monster.h"
#include "Engine/GameInstance.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "KFLikeGameInstance.generated.h"

class FOnlineSessionSettings;

UENUM(BlueprintType)
enum class EGenetalObjectType : uint8
{
	None,
	Money,
	GoreBodyParticle,
};

USTRUCT(BlueprintType)
struct FGeneralObjectData : public FTableRowBase
{
	GENERATED_BODY()

public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	EGenetalObjectType type;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString path;
};

USTRUCT(BlueprintType)
struct FPerkLevelMap
{
	GENERATED_BODY()

	UPROPERTY()
	TMap<int32, FPerkData> LevelToDataMap;
};

USTRUCT(BlueprintType)
struct FMapData : public FTableRowBase
{
	GENERATED_BODY()

public :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	int32 id;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString name;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString path;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	FString image_path;


	bool operator==(const FMapData& Other) const
	{
		return id == Other.id;
	}

	FMapData()
		: id(0)
		  , name(TEXT(""))
		  , path(TEXT(""))
		  , image_path(TEXT(""))
	{
	}
};

static const FString PlayerBlendSpacePath =
	"/Game/KF/Character/KF_Soldier_Trip/SkeletalMesh/British_Soldier1/BlendSpace";
static const FString PlayerAnimMontagePath =
	"/Game/KF/Character/KF_Soldier_Trip/SkeletalMesh/British_Soldier1/SkeletalMeshes";
static const FString WaveBGMPath =
	"/Game/KF/Sound/Wave/BGM_Wave_Cue.BGM_Wave_Cue";
static const FString BreakBGMPath =
	"/Game/KF/Sound/Break/BGM_Break_Cue.BGM_Break_Cue";
const static FName SERVER_NAME_SETTINGS_KEY(TEXT("ServerName"));
const static FName MAP_INDEX_SETTINGS_KEY(TEXT("MapIndex"));
const static FName CURRENT_WAVE_SETTINGS_KEY(TEXT("CurrentWave"));
const static FName MAX_WAVE_SETTINGS_KEY(TEXT("MaxWave"));

UCLASS()
class KILLINGFLOORLIKE_API UKFLikeGameInstance : public UAdvancedFriendsGameInstance, public IMenuInterface
{
	GENERATED_BODY()
	UKFLikeGameInstance(const FObjectInitializer& ObjectInitializer);

	virtual void Init() override;

public:
	UFUNCTION(BlueprintCallable)
	TArray<FWeaponData> GetWeaponDataArray();
	TSubclassOf<class ABaseWeapon> GetWeaponClass(int32 id);
	TSubclassOf<AActor> GetGeneralObjectClass(EGenetalObjectType ObjectType) const;
	const FWeaponData* GetWeaponData(int32 id) const;
	const FMonsterData* GetMonsterData(int32 id) const;
	const FPerkData* GetPerkData(const EPerkType PerkType, const int32 Rank) const;
	const FPerkConstData* GetPerkConstData(const EPerkType PerkType) const;
	FString GetPerkImagePath(const EPerkType PerkType, const int32 Rank) const;
	FString GetWeaponImagePath(const int32 ID) const;
	FString GetPlayerBlendSpacePath(const int32 WeaponID, bool IsSit) const;
	FString GetPlayerAnimMontagePath(const int32 WeaponID, EWeaponAnimationType WeaponAnimation) const;
	const TMap<int32, FMapData>* GetMapDataMap() const;
	const FMapData* GetMapData(int32 Id) const;
	const TArray<FMapData>* GetMapDataArray() const;
	void ShowExitMenuUI(bool IsHard = false);

	virtual void Host(FOnlineSessionSettings* SessionSettings) override;
	virtual void HostDedicatedServer(const FString& MapName, const FString& ServerName) override;
	UFUNCTION(Exec)
	virtual void Join(uint32 Index) override;

	void StartSession();

	UFUNCTION(Exec)
	virtual void LoadMainMenu() override;

	virtual void RefreshServerList() override;

private:
	UPROPERTY()
	UDataTable* GeneralObjectDataTable;
	UPROPERTY()
	UDataTable* WeaponDataTable;
	UPROPERTY()
	UDataTable* MonsterDataTable;
	UPROPERTY()
	UDataTable* PerkDataTable;
	UPROPERTY()
	UDataTable* PerkConstDataTable;
	UPROPERTY()
	UDataTable* MapDataTable;

	UPROPERTY()
	TMap<int32, TSubclassOf<class ABaseWeapon>> BaseWeaponClassMap;
	UPROPERTY()
	TMap<EGenetalObjectType, TSubclassOf<class AActor>> GeneralObjectClassMap;
	UPROPERTY()
	TMap<int32, FWeaponData> WeaponDataMap;
	UPROPERTY()
	TArray<FWeaponData> WeaponDataArray;
	UPROPERTY()
	TMap<int32, FMonsterData> MonsterDataMap;
	UPROPERTY()
	TMap<EPerkType, FPerkLevelMap> PerkDataMap;
	UPROPERTY()
	TMap<EPerkType, FPerkConstData> PerkConstDataMap;
	UPROPERTY()
	TMap<int32, FMapData> MapDataMap;
	UPROPERTY()
	TArray<FMapData> MapDataArray;
	
	void SetBaseWeaponClasses(const UDataTable* MonsterDataTable);
	void SetGeneralObjectClasses(const UDataTable* MonsterDataTable);
	void SetMonsterClasses(const UDataTable* NewMonsterDataTable);
	bool LoadDataTable (const FString& Path, UDataTable*& OutDataTable);
	void LoadWeaponDataTables();
	void LoadGeneralObjectDataTable();
	void LoadMonsterDataTable();
	void LoadPerkDataTables();
	void LoadMapDataTable();
	UFUNCTION(BlueprintCallable)
	void LoadMenuWidget();
	UFUNCTION(BlueprintCallable)
	void LoadChooseMapWidget();

	void InitDataTable();


	TSharedPtr<FOnlineSessionSearch> SessionSearch;

	UPROPERTY()
	TSubclassOf<UUserWidget> MenuClass;
	UPROPERTY()
	class UMainWidget* MainWidget;

	UPROPERTY()
	TSubclassOf<UUserWidget>ChooseMapClass;
	UPROPERTY()
	class UChooseMapWidget*ChooseMapWidget;

	UPROPERTY()
	TSubclassOf<UUserWidget> ExitClass;
	UPROPERTY()
	class UUserWidget* ExitWidget;

	void OnCreateSessionComplete(FName SessionName, bool Success);
	void OnDestroySessionComplete(FName SessionName, bool Success);
	void OnUpdateSessionComplete(FName SessionName, bool Success);
	void OnFindSessionComplete(bool Success);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);
	void OnNetworkFailure(UWorld* World, UNetDriver* NetDriver, ENetworkFailure::Type Arg, const FString& String);
	
	void CreateSession();
	UFUNCTION(BlueprintCallable)
	void CheckGameModeAndExit(APlayerController* PlayerController);
	TSharedPtr<FOnlineSessionSettings> OnlineSessionSettings;

public:
	IOnlineSessionPtr SessionInterface;
};
