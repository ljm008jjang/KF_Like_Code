// Fill out your copyright notice in the Description page of Project Settings.


#include "KillingFloorHud.h"

#include "CrossHair.h"
#include "DebugArrow.h"
#include "ShopManager.h"
#include "ShopWidget.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"

void AKillingFloorHud::BeginPlay()
{
	Super::BeginPlay();

	// 이 코드는 클라이언트에서 실행됩니다.
	if (CrosshairWidgetClass)
	{
		WBCrossHairInstance = CreateWidget<UCrossHair>(GetOwningPlayerController(), CrosshairWidgetClass);
		if (WBCrossHairInstance)
		{
			WBCrossHairInstance->AddToViewport();
		}
		else
		{
			UE_LOG(LogTemp, Error,
			       TEXT("AKillingFloorHud::BeginPlay - MainWidget 생성 실패! HUD 블루프린트에서 위젯 클래스가 할당되었는지 확인하세요."));
		}
	}

	if (ShopWidgetClass)
	{
		// 2. 위젯을 생성하고, PlayerController를 소유자로 지정
		WBShopInstance = CreateWidget<UShopWidget>(GetOwningPlayerController(), ShopWidgetClass);
	}

	if (DebugArrowClass)
	{
		FActorSpawnParameters SpawnParams;
		// 스폰되는 DebugArrow의 소유자를 이 HUD를 소유한 PlayerController로 명확하게 지정합니다.
		SpawnParams.Owner = GetOwningPlayerController();

		// 이 HUD를 소유한 로컬 클라이언트의 월드에 DebugArrow를 스폰합니다.
		DebugArrowInstance = GetWorld()->SpawnActor<ADebugArrow>(DebugArrowClass, SpawnParams);
 
		if (DebugArrowInstance)
		{
			// 월드에 있는 ShopManager를 찾아 Arrow를 초기화합니다.
			// 이 방식은 ShopManager가 월드에 하나만 존재한다고 가정합니다.
			AShopManager* ShopManager = Cast<AShopManager>(UGameplayStatics::GetActorOfClass(GetWorld(), AShopManager::StaticClass()));
			if (ShopManager)
			{
				// 이제 컨트롤러를 직접 전달할 필요 없이, Arrow가 스스로 주인을 찾아 초기화합니다.
				DebugArrowInstance->Init(ShopManager);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("AKillingFloorHud: Could not find AShopManager in the world to initialize DebugArrow."));
			}
		}
	}
}
