// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerAnimInstance.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PawnMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UPlayerAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	APawn* PawnOwner = TryGetPawnOwner();
	if (PawnOwner == nullptr)
	{
		return;
	}

	// 속도 벡터를 먼저 가져옵니다.
	FVector Velocity = PawnOwner->GetVelocity();
    
	// Z축 속도를 제거하여 수평 속도만 남깁니다.
	Velocity.Z = 0.0f;

	///Calc Speed
	UCharacterMovementComponent* MovementComponent = PawnOwner->FindComponentByClass<UCharacterMovementComponent>();
	if (MovementComponent == nullptr)
	{
		return;
	}

	float MaxSpeed = MovementComponent->GetMaxSpeed();
	if (MaxSpeed <= 0.0f)
	{
		// MaxSpeed가 0일 경우 Speed는 0으로 처리
		Speed = 0.0f;
	}
	else
	{
		int32 IsForward = FVector::DotProduct(PawnOwner->GetActorForwardVector(), Velocity) >= 0 ? 1 : -1;
		// Z가 0이 된 속도의 크기를 사용합니다.
		Speed = (Velocity.Length() / MaxSpeed) * 100 * IsForward;
	}


	///Calc Angle
	// Z가 0이 된 속도를 로컬 공간으로 변환합니다.
	FVector AngleVector = PawnOwner->GetActorTransform().InverseTransformVector(Velocity);
    
	// 방향 벡터에서 직접 각도를 계산합니다. (가장 중요한 수정)
	Angle = AngleVector.ToOrientationRotator().Yaw;
}
