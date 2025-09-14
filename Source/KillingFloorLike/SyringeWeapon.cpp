// Fill out your copyright notice in the Description page of Project Settings.


#include "SyringeWeapon.h"

#include "PubSubManager.h"

void ASyringeWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	SetLoadedAmmo( MaxMedicine);

	GetWorld()->GetTimerManager().SetTimer(MyTimerHandle, FTimerDelegate::CreateLambda([&]()
	{
		// 내가 원하는 코드 구현
		SetLoadedAmmo(GetLoadedAmmo() + RegenRate);
		if (GetLoadedAmmo() > 100)
		{
			SetLoadedAmmo(100);
		}
		else
		{
			GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Player_AmmoChange, this,
			                                                           FGameplayMessage_PlayerAmmoChange(GetOwner(), this));
		}
	}), 1, true); // 반복 실행을 하고 싶으면 false 대신 true 대입
}

void ASyringeWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(MyTimerHandle);
	Super::EndPlay(EndPlayReason);
}
