// Fill out your copyright notice in the Description page of Project Settings.


#include "KFCheckBox.h"

UKFCheckBox::UKFCheckBox(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// C++에서 기본 사운드를 설정합니다. 이 방식은 빌드 시 쿠커가 애셋을 인식하게 합니다.
	// 에디터의 블루프린트에서 다른 사운드로 언제든지 교체할 수 있습니다.
	static ConstructorHelpers::FObjectFinder<USoundBase> HoverSoundAsset(
		TEXT("/Game/KF/Sound/Other/msfxMouseOver.msfxMouseOver"));

	static ConstructorHelpers::FObjectFinder<USoundBase> ClickSoundAsset(
		TEXT("/Game/KF/Sound/Other/msfxMouseClick.msfxMouseClick"));

	// 현재 스타일을 복사한 후, 사운드 부분만 수정하고 다시 적용합니다.
	FCheckBoxStyle CheckBoxStyle = GetWidgetStyle();

	if (HoverSoundAsset.Succeeded())
	{
		FSlateSound SlateButtonPressSound = FSlateSound();
		SlateButtonPressSound.SetResourceObject(HoverSoundAsset.Object);
		CheckBoxStyle.SetHoveredSound(SlateButtonPressSound);
	}
	if (ClickSoundAsset.Succeeded())
	{
		FSlateSound SlateButtonPressSound = FSlateSound();
		SlateButtonPressSound.SetResourceObject(ClickSoundAsset.Object);
		CheckBoxStyle.SetCheckedSound(SlateButtonPressSound);
		CheckBoxStyle.SetCheckedSound(SlateButtonPressSound);
	}

	// 수정된 스타일을 버튼에 적용합니다.
	SetWidgetStyle(CheckBoxStyle);
}