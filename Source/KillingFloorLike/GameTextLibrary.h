// C:/Users/ljm/Documents/Unreal_Projects/KF_Like/Source/KillingFloorLike/GameTextLibrary.h

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameTextLibrary.generated.h"

/**
 * 게임 전체에서 사용되는 텍스트 형식을 관리하는 유틸리티 라이브러리입니다.
 */
UCLASS()
class KILLINGFLOORLIKE_API UGameTextLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * 돈의 액수를 받아 '$1,234' 형식의 FText로 변환합니다.
	 * @param Amount 돈의 액수
	 * @return 형식이 적용된 FText
	 */
	UFUNCTION(BlueprintPure, Category = "Game Text Library")
	static FText FormatMoney(int32 Amount);

	/**
	 * 현재 값과 최대 값을 받아 '75 / 100' 형식의 FText로 변환합니다.
	 * @param CurrentValue 현재 값 (예: 현재 체력)
	 * @param MaxValue 최대 값 (예: 최대 체력)
	 * @return 형식이 적용된 FText
	 */
	UFUNCTION(BlueprintPure, Category = "Game Text Library")
	static FText FormatCurrentOverMax(int32 CurrentValue, int32 MaxValue);

	/**
	 * 현재 탄약과 보유 탄약을 받아 '30 | 120' 형식의 FText로 변환합니다.
	 * @param ClipAmmo 현재 탄창의 탄약 수
	 * @param CarriedAmmo 보유 중인 총 탄약 수
	 * @return 형식이 적용된 FText
	 */
	UFUNCTION(BlueprintPure, Category = "Game Text Library")
	static FText FormatAmmo(int32 ClipAmmo, int32 CarriedAmmo);
};