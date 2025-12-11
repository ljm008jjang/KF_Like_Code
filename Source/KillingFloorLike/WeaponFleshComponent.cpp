// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponFleshComponent.h"

#include "KillingFloorLikeCharacter.h"
#include "PlayerCharacterController.h"
#include "Components/SpotLightComponent.h"

// Sets default values for this component's properties
UWeaponFleshComponent::UWeaponFleshComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// SpotLight 컴포넌트 생성
	FleshLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Flesh Light"));

	if (FleshLight)
	{
		FleshLight->SetIntensity(5000.0f); // 밝기 설정
		FleshLight->SetLightColor(FLinearColor::White); // 빛 색상
		FleshLight->SetInnerConeAngle(0); // 내부 콘 각도
		FleshLight->SetOuterConeAngle(25); // 외부 콘 각도
		FleshLight->SetAttenuationRadius(1000.0f); // 감쇠 반경
		FleshLight->SetVisibility(false); // 처음엔 꺼진 상태
	}
}


// Called when the game starts
void UWeaponFleshComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UWeaponFleshComponent::TickComponent(float DeltaTime, enum ELevelTick TickType,
                                          FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (IsValid(FleshLight) == false || FleshLight->IsVisible() == false)
	{
		return;
	}

	ABaseWeapon* BaseWeapon = Cast<ABaseWeapon>(GetOwner());
	if (IsValid(BaseWeapon) == false || IsValid(BaseWeapon->GetCharacter()) == false || IsValid(BaseWeapon->GetCharacter()->
		GetPlayerCharacterController()) == false || IsValid(BaseWeapon->GetCharacter()->
		                                                         GetPlayerCharacterController()->PlayerCameraManager) ==
		false)
	{
		return;
	}
	FRotator CameraRotation = BaseWeapon->GetCharacter()->GetPlayerCharacterController()->PlayerCameraManager->
	                                      GetCameraRotation();
	FleshLight->SetWorldRotation(CameraRotation);
}


// 플래시라이트 켜고 끄는 기능
void UWeaponFleshComponent::ToggleFlashlight(bool IsFleshOn)
{
	if (GetIsFleshOn() == IsFleshOn)
	{
		return;
	}


	if (FleshLight)
	{
		FleshLight->SetVisibility(IsFleshOn);

		if (AActor* Owner = GetOwner())
		{
			if (Owner->GetRootComponent())
			{
				FleshLight->AttachToComponent(Owner->GetRootComponent(),
				                              FAttachmentTransformRules::KeepRelativeTransform);
			}


			// 카메라의 위치와 방향을 가져오기 위해 캐릭터를 찾음
			AKillingFloorLikeCharacter* PlayerPawn = Cast<AKillingFloorLikeCharacter>(Owner->GetOwner());
			if (PlayerPawn)
			{
				//APlayerController* PlayerController = Cast<APlayerController>(PlayerPawn->GetController());
				if (PlayerPawn->GetMesh1P()->DoesSocketExist(FName("tip")))
				{
					FVector CameraLocation = PlayerPawn->GetMesh1P()->GetBoneLocation(FName("tip"));
					FRotator CameraRotation = PlayerPawn->GetMesh1P()->GetBoneQuaternion(FName("tip")).Rotator();

					// SpotLight 위치를 카메라 정면으로 설정
					FleshLight->SetWorldLocation(CameraLocation + CameraRotation.Vector() * .0f); // 100cm 앞
					FleshLight->SetWorldRotation(CameraRotation);
				}
			}
		}
	}
}

bool UWeaponFleshComponent::GetIsFleshOn()
{
	return FleshLight->IsVisible();
}

UAnimMontage* UWeaponFleshComponent::GetFleshAnimMontage()
{
	return FleshAnimMontage;
}
