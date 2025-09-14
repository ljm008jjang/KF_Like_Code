// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "KillingFloorHud.generated.h"

class ADebugArrow;
class UShopWidget;
class AKillingFloorLikeGameState;
class UCrossHair;
/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API AKillingFloorHud : public AHUD
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

private:
	// 디자이너가 블루프린트에서 설정할 크로스헤어 위젯 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> CrosshairWidgetClass;
	UPROPERTY(EditDefaultsOnly, Category = "Widgets")
	TSubclassOf<UUserWidget> ShopWidgetClass;

	// 생성된 Crosshair 위젯 인스턴스
	UPROPERTY()
	UCrossHair* WBCrossHairInstance;
	// 생성된 Crosshair 위젯 인스턴스
	UPROPERTY()
	UShopWidget* WBShopInstance;

	UPROPERTY(EditDefaultsOnly, Category = "HUD")
	TSubclassOf<ADebugArrow> DebugArrowClass;
	/** 스폰된 DebugArrow 인스턴스에 대한 참조입니다. */
	UPROPERTY()
	TObjectPtr<ADebugArrow> DebugArrowInstance;

public:
	UCrossHair* GetWbCrossHairInstance() const
	{
		return WBCrossHairInstance;
	}

	UShopWidget* GetWbShopInstance() const
	{
		return WBShopInstance;
	}
};
