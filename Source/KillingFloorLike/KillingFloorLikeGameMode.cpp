// Copyright Epic Games, Inc. All Rights Reserved.

#include "KillingFloorLikeGameMode.h"

#include "KFLikeGameInstance.h"
#include "KillingFloorLikeCharacter.h"
#include "KillingFloorLikeGameState.h"
#include "NavigationSystem.h"
#include "OnlineSessionSettings.h"
#include "PubSubManager.h"
#include "ShopManager.h"
#include "SoundManager.h"
#include "UnitManager.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

void AKillingFloorLikeGameMode::BeginPlay()
{
	Super::BeginPlay();

	NavSystem = UNavigationSystemV1::GetNavigationSystem(GetWorld());

	CachedGameState = GetGameState<AKillingFloorLikeGameState>();

	if (USoundManager* SoundManager = GetGameInstance()->GetSubsystem<USoundManager>())
	{
		SoundManager->Init();
	}

	GetGameInstance()->GetSubsystem<UPubSubManager>()->Subscribe(EPubSubTag::Mode_MonsterCountChange, this,
	                                                             &AKillingFloorLikeGameMode::CheckWaveStateMonsterCount);
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Subscribe(EPubSubTag::Player_Dead, this,
	                                                             &AKillingFloorLikeGameMode::CheckWaveStatePlayerDead);

	OnGameStartEvent(10);
}

void AKillingFloorLikeGameMode::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(ZedTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(GameStartTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(GameEndTimerHandle);
	Super::EndPlay(EndPlayReason);
}

AKillingFloorLikeGameMode::AKillingFloorLikeGameMode()
	: Super()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AKillingFloorLikeGameMode::StartWave()
{
	// 2. 플레이어 설정
	SetAllPlayersChangablePerk(false);

	USoundManager* SoundManager = USoundManager::GetSoundManager(this);
	// 3. 배경음 변경 (TODO: 향후 DB 또는 설정값으로 관리)
	if (IsValid(SoundManager))
	{
		SoundManager->Multi_ChangeBGM(WaveBGMPath);
	}

	// 4. 웨이브 타이머 시작
	WaveDelayTime = 0.0f;
	GetWorldTimerManager().SetTimer(
		CountdownTimerHandle,
		this,
		&AKillingFloorLikeGameMode::UpdateWaveTimeCountdown,
		1.0f,
		true
	);

	// 5. 유닛 풀 및 데이터베이스 초기화
	if (CachedGameState && CachedGameState->GetUnitManager())
	{
		CachedGameState->GetUnitManager()->ClearUnitDB();
		CachedGameState->GetUnitManager()->RefillMonsterPool(CachedGameState->GetCurrentWave());
	}

	// 6. 모드 전환 및 이벤트 브로드캐스트
	ChangeModeType(EModeType::Wave);

	// 7. 디버그 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("Start Wave"));
}

void AKillingFloorLikeGameMode::UpdateWaveTimeCountdown()
{
	WaveDelayTime = FMath::Max(WaveDelayTime - 1, 0);

	if (CachedGameState->GetUnitManager()->GetSpawnedMonsterCount(true) <= 0 || WaveDelayTime <= 0)
	{
		CachedGameState->GetUnitManager()->SpawnMonster();
		WaveDelayTime = MaxWaveTime;
	}
}

void AKillingFloorLikeGameMode::CheckWaveStateMonsterCount(FGameplayTag Channel,
                                                           const FGameplayMessage_MonsterCount& Message)
{
	if (CachedGameState->GetCurrentModeType() == EModeType::Wave)
	{
		if (Message.RemainMonsterCount <= 0)
		{
			EndWave(true);
		}
	}
}

void AKillingFloorLikeGameMode::CheckWaveStatePlayerDead(FGameplayTag Channel, const FGameplayMessage_None& Message)
{
	if (IsValid(CachedGameState) == false)
	{
		return;
	}
	
	if (CachedGameState->GetCurrentModeType() == EModeType::Wave)
	{
		// GameState의 PlayerArray를 순회하며 살아있는 플레이어가 있는지 확인합니다.
		for (APlayerState* PlayerState : CachedGameState->PlayerArray)
		{
			AKillingFloorLikeCharacter* KFCharacter = Cast<AKillingFloorLikeCharacter>(PlayerState->GetPawn());
			if (IsValid(KFCharacter) == false)
			{
				continue;
			}
			// PlayerState가 유효하고, 해당 플레이어가 관전 상태가 아니라면 (즉, 아직 살아있다면)
			// 즉시 함수를 종료합니다. 아직 게임이 끝나지 않았습니다.
			if (KFCharacter->GetCurrentUnitState() != EUnitState::Dead)
			{
				return;
			}
		}

		// 루프를 모두 통과했다는 것은 살아있는 플레이어가 한 명도 없다는 의미입니다.
		// 웨이브를 패배로 종료합니다.
		UE_LOG(LogTemp, Log, TEXT("All players are dead. Ending wave."));
		EndWave(false);
	}
}

void AKillingFloorLikeGameMode::EndWave(bool IsWin)
{
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	if (IsWin)
	{
		if (CachedGameState->GetCurrentWave() >= CachedGameState->MaxWaveIndex)
		{
			EndMatch(true);
			return;
		}

		StartBreak();
	}
	else
	{
		EndMatch(false);
	}
}

void AKillingFloorLikeGameMode::StartBreak()
{
	// 1. 디버그 로그 출력
	UE_LOG(LogTemp, Warning, TEXT("Start Break"));

	// 모든 죽은 플레이어를 부활시킵니다.
	if (CachedGameState)
	{
		CachedGameState->SetCurrentWave(CachedGameState->GetCurrentWave() + 1);
		
		for (APlayerState* PlayerState : CachedGameState->PlayerArray)
		{
			if (IsValid(PlayerState) == false) continue;

			APawn* CurrentPawn = PlayerState->GetPawn();
			AKillingFloorLikeCharacter* KFCharacter = Cast<AKillingFloorLikeCharacter>(CurrentPawn);

			// 플레이어가 폰을 가지고 있지 않거나, 캐릭터가 죽은 상태라면 리스폰시킵니다.
			if (IsValid(CurrentPawn) == false || (IsValid(KFCharacter) && KFCharacter->GetCurrentUnitState() == EUnitState::Dead))
			{
				if (AController* PlayerController = PlayerState->GetOwner<AController>())
				{
					RestartPlayer(PlayerController);
					KFCharacter->Multi_RestartCallback();
				}
			}
		}
	}

	// 2. 배경음 변경 (TODO: 설정 파일 또는 데이터 자산화 필요)
	USoundManager* SoundManager = USoundManager::GetSoundManager(this);
	// 3. 배경음 변경 (TODO: 향후 DB 또는 설정값으로 관리)
	if (SoundManager)
	{
		SoundManager->Multi_ChangeBGM(BreakBGMPath);
	}
	//ChangeBGM(TEXT("/Game/KF/Sound/Break/BGM_Break_Cue.BGM_Break_Cue"));

	// 3. 플레이어 설정
	SetAllPlayersChangablePerk(true);

	// 4. 게임 모드 상태 전환
	ChangeModeType(EModeType::Break);

	// 6. 브레이크 시작 이벤트 브로드캐스트
	OnStartBreakEvent();
}

void AKillingFloorLikeGameMode::EndBreak()
{
	GetWorldTimerManager().ClearTimer(CountdownTimerHandle);
	CachedGameState->GetShopManager()->SetNewShop();
	StartWave();
}

void AKillingFloorLikeGameMode::EndMatch(bool IsWin)
{
	UE_LOG(LogTemp, Warning, TEXT("End Match : %hhd"), IsWin);
	CachedGameState->SetIsWin(IsWin);
	ChangeModeType(EModeType::End);
	FTimerHandle EndTimeHandler;

	//TODO 개발 필요
	if (IsWin)
	{
		// GetWorldTimerManager().ClearTimer(GameEndTimerHandle);
		// GetWorld()->GetTimerManager().SetTimer(
		// 	GameEndTimerHandle,
		// 	this,
		// 	&AKillingFloorLikeGameMode::EndBreak,
		// 	GameEndDuration,
		// 	false
		// );
	}
	else
	{
	}
}

void AKillingFloorLikeGameMode::ChangeModeType(EModeType NewModeType)
{
	CachedGameState->SetCurrentModeType(NewModeType);
}

/*
void AKillingFloorLikeGameMode::ChangeBGM(const FString& AssetPath)
{
	float FadeDuration = 2;
	BGMComponent->FadeOut(FadeDuration, 0.0f);
	// Delay 후 새 사운드 재생 (Fade In)
	FTimerHandle TimerHandle;
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, [=, this]()
	{
		BGMComponent->
			SetSound(GetGameInstance()->GetSubsystem<UResourceManager>()->LoadSound(AssetPath));
		BGMComponent->FadeIn(FadeDuration, 1.0f); // Target Volume = 1.0
	}, FadeDuration, false);
}
*/

void AKillingFloorLikeGameMode::SetAllPlayersChangablePerk(bool bCanChange)
{
	// GameState가 유효한지 먼저 확인합니다.
	if (IsValid(CachedGameState) == false)
	{
		return;
	}
	// GameState의 PlayerArray를 순회하며 모든 플레이어에게 접근합니다.
	for (APlayerState* PlayerState : CachedGameState->PlayerArray)
	{
		if (IsValid(PlayerState))
		{
			// PlayerState로부터 해당 플레이어가 조종하는 캐릭터(Pawn)를 가져옵니다.
			AKillingFloorLikeCharacter* Character = Cast<AKillingFloorLikeCharacter>(PlayerState->GetPawn());
			if (Character)
			{
				// 각 플레이어 캐릭터의 상태를 변경합니다.
				Character->SetIsChangablePerk(bCanChange);
				if (bCanChange == false)
				{
					Character->SetIsShopable(bCanChange);
				}
			}
		}
	}
}

void AKillingFloorLikeGameMode::StartZedTime(const float Duration, const float SlowRate)
{
	// 이 함수는 반드시 서버에서만 호출되어야 합니다.
	if (!HasAuthority())
	{
		return;
	}

	// 1. 실제 게임 월드의 시간을 느리게 만듭니다. 이 변경사항은 엔진에 의해 클라이언트에 적용됩니다.
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), SlowRate);

	// 2. 캐싱해 둔 GameState의 상태 변수를 변경합니다.
	//    이 값을 변경하면 모든 클라이언트에서 OnRep_CurrentTimeDilation 함수가 호출됩니다.
	if (GetCachedGameState()) // MyGameState는 BeginPlay 등에서 캐싱해두어야 합니다.
	{
		GetCachedGameState()->CurrentTimeDilation = SlowRate;
		// 서버 자신도 OnRep을 호출해주어야 일관성 있는 동작이 가능합니다.
		GetCachedGameState()->OnRep_CurrentTimeDilation();
	}

	// 3. 타이머를 설정하여 원래 시간으로 되돌립니다.
	GetWorld()->GetTimerManager().ClearTimer(ZedTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(ZedTimerHandle, this, &AKillingFloorLikeGameMode::StopZedTime, Duration,
	                                       false);

	/*if (UWorld* World = GetWorld())
	{
		UGameplayStatics::SetGlobalTimeDilation(World, SlowRate);

		UGameplayStatics::PlaySound2D(
			GetWorld(),
			GetGameInstance()->GetSubsystem<UResourceManager>()->LoadSound(
				"/Game/KF/Sound/Other/Slow_Motion_Sound_Effect.Slow_Motion_Sound_Effect"));

		FTimerHandle TimerHandle;
		World->GetTimerManager().SetTimer(TimerHandle, [World]()
		{
			UGameplayStatics::SetGlobalTimeDilation(World, 1.0f);
		}, Duration, false);
	}*/
}

void AKillingFloorLikeGameMode::StopZedTime()
{
	// 1. 시간을 원래대로 되돌립니다.
	UGameplayStatics::SetGlobalTimeDilation(GetWorld(), 1.0f);

	// 2. GameState의 상태도 원래대로 변경하여 모두에게 알립니다.
	if (GetCachedGameState())
	{
		GetCachedGameState()->CurrentTimeDilation = 1.0f;
		GetCachedGameState()->OnRep_CurrentTimeDilation();
	}
}

void AKillingFloorLikeGameMode::UpdateSessionWaveInfo(int32 NewWave)
{
	if (HasAuthority() == false)
	{
		return;
	}

	UKFLikeGameInstance* GameInstance = Cast<UKFLikeGameInstance>(GetGameInstance());
	if (GameInstance && GameInstance->SessionInterface.IsValid())
	{
		// 현재 세션의 설정을 가져옵니다.
		FOnlineSessionSettings* CurrentSettings = GameInstance->SessionInterface->GetSessionSettings(NAME_GameSession);
		if (CurrentSettings)
		{
			// 웨이브 정보를 새로운 값으로 갱신합니다.
			CurrentSettings->Set(CURRENT_WAVE_SETTINGS_KEY, NewWave,
			                     EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);
			// 변경된 설정으로 세션을 업데이트합니다.
			GameInstance->SessionInterface->UpdateSession(NAME_GameSession, *CurrentSettings, true);
		}
	}
}


void AKillingFloorLikeGameMode::OnStartBreakEvent()
{
	// 이 로직은 반드시 서버에서만 실행되어야 합니다.
	if (HasAuthority() && CachedGameState)
	{
		// 2. 서버의 게임 로직을 위해 "EndBreak" 함수를 예약합니다.
		// 이전 타이머가 남아있을 수 있으니 항상 먼저 초기화합니다.
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);

		// BreakDuration(60초) 후에 EndBreak 함수를 딱 한 번 호출하도록 타이머를 설정합니다.
		// 세 번째 인자(InRate)는 반복 시간, 네 번째 인자(InbLoop)는 반복 여부입니다. false로 설정합니다.
		GetWorld()->GetTimerManager().SetTimer(
			CountdownTimerHandle,
			this,
			&AKillingFloorLikeGameMode::EndBreak,
			BreakDuration,
			false
		);
		// 2. GameState의 NetMulticast RPC를 호출합니다.
		//    이 함수는 서버 자신과 모든 클라이언트에서 실행됩니다.
		//    필요하다면 MonsterCount를 파라미터로 넘겨줄 수 있습니다.
		// "남은 시간"이 아닌 "종료될 시각"을 계산하여 GameState에 단 한번만 설정합니다.
		const float EndTime = GetWorld()->GetTimeSeconds() + BreakDuration;
		//CachedGameState->BreakEndTime = EndTime;
		CachedGameState->SetBreakEndTime(EndTime);

		// 서버 자신도 OnRep을 호출하여 로직을 일관성 있게 만듭니다.
		//CachedGameState->OnRep_BreakEndTime();
		//CachedGameState->Multicast_OnBreakStart();
	}
}

void AKillingFloorLikeGameMode::OnGameStartEvent(float BreakDurationVal)
{
	// 이 로직은 반드시 서버에서만 실행되어야 합니다.
	if (HasAuthority() && CachedGameState)
	{
		// 2. 서버의 게임 로직을 위해 "EndBreak" 함수를 예약합니다.
		// 이전 타이머가 남아있을 수 있으니 항상 먼저 초기화합니다.
		GetWorld()->GetTimerManager().ClearTimer(CountdownTimerHandle);

		// BreakDuration(60초) 후에 EndBreak 함수를 딱 한 번 호출하도록 타이머를 설정합니다.
		// 세 번째 인자(InRate)는 반복 시간, 네 번째 인자(InbLoop)는 반복 여부입니다. false로 설정합니다.
		GetWorld()->GetTimerManager().SetTimer(
			CountdownTimerHandle,
			this,
			&AKillingFloorLikeGameMode::EndBreak,
			BreakDurationVal,
			false
		);
		// 2. GameState의 NetMulticast RPC를 호출합니다.
		//    이 함수는 서버 자신과 모든 클라이언트에서 실행됩니다.
		//    필요하다면 MonsterCount를 파라미터로 넘겨줄 수 있습니다.
		// "남은 시간"이 아닌 "종료될 시각"을 계산하여 GameState에 단 한번만 설정합니다.
		const float EndTime = GetWorld()->GetTimeSeconds() + BreakDurationVal;
		//CachedGameState->BreakEndTime = EndTime;
		CachedGameState->SetBreakEndTime(EndTime);
		// 서버 자신도 OnRep을 호출하여 로직을 일관성 있게 만듭니다.
		//CachedGameState->OnRep_BreakEndTime();
		//CachedGameState->Multicast_OnGameStart();
	}
}
