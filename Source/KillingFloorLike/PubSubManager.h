// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "KillingFloorLikeCharacter.h"
#include "KillingFloorLikeGameState.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "PubSubManager.generated.h"

UENUM(BlueprintType)
enum class EPubSubTag : uint8
{
	None,
	//가지고 있는 무기 자체가 변경되면
	Player_WeaponChange UMETA(GameplayTag = "Character.Player.WeaponChange"),
	Player_PerkChange UMETA(GameplayTag = "Character.Player.PerkChange"),
	Player_AmmoChange UMETA(GameplayTag = "Character.Player.AmmoChange"),
	Player_MoneyChange UMETA(GameplayTag = "Character.Player.MoneyChange"),
	Player_HealthChange UMETA(GameplayTag = "Character.Player.HealthChange"),
	Player_Dead UMETA(GameplayTag = "Character.Player.Dead"),

	Mode_End UMETA(GameplayTag = "Game.Mode.End"),
	Mode_ModeTypeChanged UMETA(GameplayTag = "Game.Mode.ModeTypeChanged"),
	Mode_BreakEndTimeChanged UMETA(GameplayTag = "Game.Mode.Mode_BreakEndTimeChanged"),
	Mode_MonsterCountChange UMETA(GameplayTag = "Game.Mode.MonsterCountChange"),
};

USTRUCT(BlueprintType)
struct FGameplayMessage_None
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> ListenerObject;
};

USTRUCT(BlueprintType)
struct FGameplayMessage_PlayerAmmoChange
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> ListenerObject;

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<ABaseWeapon> Weapon;
};

USTRUCT(BlueprintType)
struct FGameplayMessage_PlayerMoneyChange
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> ListenerObject;
	UPROPERTY(BlueprintReadWrite)
	int32 Money;
};

USTRUCT(BlueprintType)
struct FGameplayMessage_PlayerHeathChange
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> ListenerObject;
	UPROPERTY(BlueprintReadWrite)
	float Health;
	UPROPERTY(BlueprintReadWrite)
	float Armor;
};

USTRUCT(BlueprintType)
struct FGameplayMessage_Perk
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> ListenerObject;
	UPROPERTY(BlueprintReadWrite)
	FPerkData PerkData;
};

USTRUCT(BlueprintType)
struct FGameplayMessage_MonsterCount
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> ListenerObject;
	UPROPERTY(BlueprintReadWrite)
	int32 RemainMonsterCount;
};

USTRUCT(BlueprintType)
struct FGameplayMessage_GameMode
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> ListenerObject;
	UPROPERTY(BlueprintReadWrite)
	EModeType ModeType;
};

USTRUCT(BlueprintType)
struct FGameplayMessage_BreakEndTimeChanged
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> ListenerObject;
	UPROPERTY(BlueprintReadWrite)
	int32 BreakEndTime;
};

USTRUCT(BlueprintType)
struct FGameplayMessage_EndMatch
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite)
	TWeakObjectPtr<UObject> ListenerObject;
	UPROPERTY(BlueprintReadWrite)
	bool IsWin;
};

USTRUCT(BlueprintType)
struct FPubSubPayload
{
	GENERATED_BODY()

	virtual ~FPubSubPayload() = default;
};

// 이 델리게이트를 통해 모든 메시지가 전달됩니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPubSubMessage, const FPubSubPayload&, Payload);

UCLASS()
class KILLINGFLOORLIKE_API UPubSubManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** UGameInstanceSubsystem의 생명주기 함수: 서브시스템이 초기화될 때 호출됩니다. */
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** UGameInstanceSubsystem의 생명주기 함수: 서브시스템이 소멸될 때 호출됩니다. */
	virtual void Deinitialize() override;

	template<typename T, typename U>
	void Publish(EPubSubTag Channel, T* ListenerObject, const U& Payload)
	{
		if (!IsValid(ListenerObject)) return;

		const FGameplayTag GameplayTagToBroadcast = GetTagFromChannel(Channel);
		if (!GameplayTagToBroadcast.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Broadcast failed: Could not find a valid GameplayTag for the given enum."));
			return;
		}

		const FString ChannelName = StaticEnum<EPubSubTag>()->GetNameStringByValue((int64)Channel);
		//UE_LOG(LogTemp, Warning, TEXT("Broadcast success : %s"), *ChannelName);

		// GameplayMessageSubsystem을 통해 메시지를 실제로 발행합니다.
		auto& MessageSubsystem = UGameplayMessageSubsystem::Get(ListenerObject);
		MessageSubsystem.BroadcastMessage(GameplayTagToBroadcast, Payload);
	}

	/**
	 * 외부 객체에서 리스너 구독을 요청하기 위한 함수입니다.
	 * 핸들 관리를 신경 쓸 필요 없이 이 함수만 호출하면 됩니다.
	 * @param Tag 구독할 메시지 채널 (게임플레이 태그)
	 * @param ListenerObject 메시지를 수신할 객체 (보통 this)
	 * @param Func 메시지를 받았을 때 호출될 콜백 함수
	 */
	template<typename T, typename U>
	void Subscribe(EPubSubTag Tag, T* ListenerObject, void (T::*Func)(FGameplayTag, const U&))
	{
		if (!IsValid(ListenerObject)) return;

		// 1. Enum을 GameplayTag로 변환합니다.
		const FGameplayTag GameplayTagToRegister = GetTagFromChannel(Tag);

		if (!GameplayTagToRegister.IsValid())
		{
			UE_LOG(LogTemp, Warning, TEXT("Subscribe failed: Could not find a valid GameplayTag for the given enum."));
			return;
		}
		
		// 실제 리스너 등록은 GameplayMessageSubsystem에게 위임합니다.
		auto& MessageSubsystem = UGameplayMessageSubsystem::Get(ListenerObject);
		
		FGameplayMessageListenerHandle Handle = MessageSubsystem.RegisterListener(GameplayTagToRegister, ListenerObject, Func);
        
		// [디버깅 로그] 어떤 객체가 어떤 채널에 구독했는지 기록합니다.
		//UE_LOG(LogTemp, Log, TEXT("Subscribe success: Object '%s' subscribed to channel '%s'"), *GetNameSafe(ListenerObject), *GameplayTagToRegister.ToString());

		// 반환된 핸들을 객체와 짝을 지어 내부 맵에 저장합니다.
		ListenerHandlesMap.FindOrAdd(ListenerObject).Add(Handle);
	}

	UFUNCTION(BlueprintPure, Category = "PubSub")
	FGameplayTag GetTagFromChannel(EPubSubTag Channel);

	void Unsubscribe(UObject* ListenerObject);

private:
	/** 가비지 컬렉션이 시작되기 직전에 호출될 콜백 함수입니다. */
	//void OnPreGarbageCollect();

	/**
	 * 리스너들을 관리하기 위한 핵심 데이터 구조입니다.
	 * 키: 리스너 객체를 가리키는 약한 포인터(TWeakObjectPtr)
	 * 값: 해당 객체가 등록한 모든 리스너 핸들의 배열
	 */
	TMap<TWeakObjectPtr<UObject>, TArray<FGameplayMessageListenerHandle>> ListenerHandlesMap;
	/** EPubSubTag와 FGameplayTag를 런타임에 매핑하기 위한 TMap 컨테이너입니다. */
	/** 델리게이트 바인딩을 추적하기 위한 핸들입니다. #1#
	FDelegateHandle OnPreGarbageCollectHandle;*/
	UPROPERTY()
	TMap<EPubSubTag, FGameplayTag> TagChannelMap;
};