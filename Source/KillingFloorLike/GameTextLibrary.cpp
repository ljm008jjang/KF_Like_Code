// C:/Users/ljm/Documents/Unreal_Projects/KF_Like/Source/KillingFloorLike/GameTextLibrary.cpp

#include "GameTextLibrary.h"

// 지역화 텍스트를 위한 네임스페이스를 정의합니다.
#define LOCTEXT_NAMESPACE "GameTextLibrary"

FText UGameTextLibrary::FormatMoney(int32 Amount)
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Amount"), FText::AsNumber(Amount));
	
	// LOCTEXT를 사용하여 나중에 텍스트를 다른 언어로 쉽게 번역할 수 있습니다.
	return FText::Format(LOCTEXT("MoneyFormat", "${Amount}"), Args);
}

FText UGameTextLibrary::FormatCurrentOverMax(int32 CurrentValue, int32 MaxValue)
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Current"), FText::AsNumber(CurrentValue));
	Args.Add(TEXT("Max"), FText::AsNumber(MaxValue));

	return FText::Format(LOCTEXT("CurrentOverMaxFormat", "{Current} / {Max}"), Args);
}

FText UGameTextLibrary::FormatAmmo(int32 ClipAmmo, int32 CarriedAmmo)
{
	FFormatNamedArguments Args;
	Args.Add(TEXT("Clip"), FText::AsNumber(ClipAmmo));
	Args.Add(TEXT("Carried"), FText::AsNumber(CarriedAmmo));

	return FText::Format(LOCTEXT("AmmoFormat", "{Clip} | {Carried}"), Args);
}

// 네임스페이스 정의를 해제합니다.
#undef LOCTEXT_NAMESPACE