// Fill out your copyright notice in the Description page of Project Settings.


#include "KillingFloorLikeGameState.h"

#include "KillingFloorLikeGameMode.h"
#include "PubSubManager.h"
#include "ResourceManager.h"
#include "ShopManager.h"
#include "SoundManager.h"
#include "UnitManager.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void AKillingFloorLikeGameState::BeginPlay()
{
	Super::BeginPlay();

	SetCurrentWave(1);

	UnitManager = Cast<AUnitManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AUnitManager::StaticClass()));
	ShopManager = Cast<AShopManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AShopManager::StaticClass()));
	ZedTimeStartSound = GetGameInstance()->GetSubsystem<UResourceManager>()->LoadSound(
		"/Game/KF/Sound/Other/Slow_Motion_Sound_Effect.Slow_Motion_Sound_Effect");
}

void AKillingFloorLikeGameState::OnRep_CurrentModeType()
{
	UE_LOG(LogTemp,Warning,TEXT("OnRep_CurrentModeType : %d"), CurrentModeType);
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Mode_ModeTypeChanged,this,
	                                                           FGameplayMessage_GameMode(
		                                                           this, CurrentModeType));

	if (CurrentModeType == EModeType::End)
	{
		GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Mode_End,this,
															   FGameplayMessage_EndMatch(
																   this, IsWin));
	}
}

void AKillingFloorLikeGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// CurrentTimeDilation 변수를 모든 클라이언트에게 복제합니다.
	DOREPLIFETIME(AKillingFloorLikeGameState, CurrentModeType);
	DOREPLIFETIME(AKillingFloorLikeGameState, MaxWaveIndex);
	DOREPLIFETIME(AKillingFloorLikeGameState, CurrentWave);
	DOREPLIFETIME(AKillingFloorLikeGameState, CurrentTimeDilation);
	DOREPLIFETIME(AKillingFloorLikeGameState, BreakEndTime);
	DOREPLIFETIME(AKillingFloorLikeGameState, IsWin);
}

void AKillingFloorLikeGameState::OnRep_BreakEndTime()
{
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Mode_BreakEndTimeChanged,this,
	                                                           FGameplayMessage_BreakEndTimeChanged(
		                                                           this, BreakEndTime));
}

void AKillingFloorLikeGameState::OnRep_CurrentWave()
{
	if (HasAuthority())
	{
		Cast<AKillingFloorLikeGameMode>(UGameplayStatics::GetGameMode(GetWorld()))->UpdateSessionWaveInfo(CurrentWave);
	}
}

// 이 함수는 클라이언트에서 CurrentTimeDilation 값이 변경될 때마다 자동으로 호출됩니다.
void AKillingFloorLikeGameState::OnRep_CurrentTimeDilation()
{
	// 이전 값과 비교하여 슬로우 모션이 "시작"되는 순간인지 확인합니다.
	if (CurrentTimeDilation < 1.0f)
	{
		// 클라이언트에서 효과음을 재생합니다!
		// 여기서 포스트 프로세스 이펙트를 활성화할 수도 있습니다.
		if (ZedTimeStartSound)
		{
			GetGameInstance()->GetSubsystem<USoundManager>()->Play2DSound(ZedTimeStartSound);
			//UGameplayStatics::PlaySound2D(GetWorld(), ZedTimeStartSound);
		}
	}

	// 참고: 실제 SetGlobalTimeDilation은 서버에서 직접 제어하므로
	// 클라이언트의 OnRep 함수에서 또 호출할 필요는 없습니다.
	// 이 함수는 순수하게 "클라이언트 측 효과"를 위한 것입니다.
}

FText AKillingFloorLikeGameState::GetWaveText()
{
	return FText::FromString("Wave : " + FString::FromInt(GetCurrentWave()));
}
