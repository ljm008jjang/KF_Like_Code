// Fill out your copyright notice in the Description page of Project Settings.


#include "PubSubManager.h"

void UPubSubManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	// 서브시스템이 초기화될 때, Enum과 GameplayTag의 관계를 TMap에 미리 정의합니다.
	// 이 방식은 UMETA 정보를 런타임에 읽을 수 없는 패키징 빌드 환경에서 필수적입니다.
    
	// 주의: EPubSubTag::None은 보통 유효하지 않은 값으로 취급되므로 맵에 추가하지 않습니다.
	TagChannelMap.Add(EPubSubTag::Player_WeaponChange, FGameplayTag::RequestGameplayTag(FName("Character.Player.WeaponChange")));
	TagChannelMap.Add(EPubSubTag::Player_PerkChange, FGameplayTag::RequestGameplayTag(FName("Character.Player.PerkChange")));
	TagChannelMap.Add(EPubSubTag::Player_AmmoChange, FGameplayTag::RequestGameplayTag(FName("Character.Player.AmmoChange")));
	TagChannelMap.Add(EPubSubTag::Player_MoneyChange, FGameplayTag::RequestGameplayTag(FName("Character.Player.MoneyChange")));
	TagChannelMap.Add(EPubSubTag::Player_HealthChange, FGameplayTag::RequestGameplayTag(FName("Character.Player.HealthChange")));
	TagChannelMap.Add(EPubSubTag::Player_Dead, FGameplayTag::RequestGameplayTag(FName("Character.Player.Dead")));

	TagChannelMap.Add(EPubSubTag::Mode_End, FGameplayTag::RequestGameplayTag(FName("Game.Mode.End")));
	TagChannelMap.Add(EPubSubTag::Mode_ModeTypeChanged, FGameplayTag::RequestGameplayTag(FName("Game.Mode.ModeTypeChanged")));
	TagChannelMap.Add(EPubSubTag::Mode_BreakEndTimeChanged, FGameplayTag::RequestGameplayTag(FName("Game.Mode.Mode_BreakEndTimeChanged")));
	TagChannelMap.Add(EPubSubTag::Mode_MonsterCountChange, FGameplayTag::RequestGameplayTag(FName("Game.Mode.MonsterCountChange")));

	UE_LOG(LogTemp, Log, TEXT("PubSubManager Initialized and TagChannelMap populated."));
	
	UE_LOG(LogTemp, Log, TEXT("UPubSubManager Initialized."));
}

void UPubSubManager::Deinitialize()
{
	UE_LOG(LogTemp, Log, TEXT("UPubSubManager Deinitialized."));
	Super::Deinitialize();
}

void UPubSubManager::Unsubscribe(UObject* ListenerObject)
{
	if (!IsValid(ListenerObject)) return;

	// 맵에서 해당 객체가 등록한 핸들 배열을 찾습니다.
	if (TArray<FGameplayMessageListenerHandle>* Handles = ListenerHandlesMap.Find(ListenerObject))
	{
		//UE_LOG(LogTemp, Log, TEXT("Unsubscribing all listeners for Object '%s'"), *GetNameSafe(ListenerObject));
		for (FGameplayMessageListenerHandle& Handle : *Handles)
		{
			Handle.Unregister();
		}
		ListenerHandlesMap.Remove(ListenerObject);
	}
}

FGameplayTag UPubSubManager::GetTagFromChannel(EPubSubTag Channel)
{
	const FGameplayTag* FoundTag = TagChannelMap.Find(Channel);

	if (FoundTag)
	{
		return *FoundTag;
	}

	// 맵에 없는 채널이 요청된 경우, 경고 로그를 남기고 유효하지 않은 태그를 반환합니다.
	UE_LOG(LogTemp, Warning, TEXT("GetTagFromChannel: Could not find a mapped tag for the given EPubSubTag."));
	return FGameplayTag::EmptyTag;
	/*const UEnum* EnumPtr = StaticEnum<EPubSubTag>();
	if (!EnumPtr) return FGameplayTag();

	// 1. 'Channel' 변수(열거형 값)를 사용하여 Enum 내에서의 인덱스를 찾습니다.
	const int32 EnumIndex = EnumPtr->GetIndexByValue((int64)Channel);

	// 2. 찾은 인덱스를 사용하여 해당 값에 연결된 메타데이터를 가져옵니다.
	FString TagString = EnumPtr->GetMetaData(TEXT("GameplayTag"), EnumIndex);
	return FGameplayTag::RequestGameplayTag(FName(*TagString));*/
}