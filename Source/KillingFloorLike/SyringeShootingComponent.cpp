// Fill out your copyright notice in the Description page of Project Settings.


#include "SyringeShootingComponent.h"

#include "BaseWeapon.h"
#include "DebugManagerComponent.h"
#include "HealDamageType.h"
#include "KillingFloorLikeCharacter.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

class UPubSubManager;

bool USyringeShootingComponent::Fire(AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial)
{
	if (!Character || !Character->GetController())
	{
		return false;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Character->GetInstigatorController());
	if (!PlayerController)
	{
		return false;
	}
	
	if (IsSpecial)
	{
		FDamageEvent Event;
		Event.DamageTypeClass = UHealDamageType::StaticClass();
		Character->TakeDamage(WeaponData->special_damage, Event, PlayerController, Character);
		return true;
	}
	
	const FVector Start = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FRotator CameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	const FVector ForwardVec = CameraRotation.Vector();
	const float Length = AttackRange;

	const int32 Segments = 11;
	const float Angle = 90.0f;
	const float SegmentAngle = Angle / (Segments - 1);
	const float HalfAngle = Angle * 0.5f;

	// 가장 가까운 캐릭터와 충돌 정보를 저장할 변수들을 초기화합니다.
	AKillingFloorLikeCharacter* ClosestHitCharacter = nullptr;
	FHitResult ClosestHitResult;
	float MinDistanceSq = FLT_MAX; // 거리 비교 시 제곱근 연산을 피하기 위해 제곱 거리를 사용합니다.

	// 부채꼴 레이캐스트
	for (int32 i = 0; i < Segments; ++i)
	{
		const float ThisAngle = HalfAngle - SegmentAngle * i;
		const FVector RotationAxis = CameraRotation.Quaternion().GetUpVector();
		const FQuat RotationQuat = FQuat(RotationAxis, FMath::DegreesToRadians(ThisAngle));

		FVector End = RotationQuat.RotateVector(ForwardVec) * Length + Start;

		// 디버그 라인 표시 (힐링 효과이므로 초록색으로 변경)
		UDebugManagerComponent::DrawDebugLineManager(GetWorld(), Start, End, FColor::Green, false, 5.0f);

		TArray<FHitResult> HitResults;
		FCollisionObjectQueryParams ObjectQueryParams;
		ObjectQueryParams.AddObjectTypesToQuery(ECC_Pawn);

		bool bHit = GetWorld()->LineTraceMultiByObjectType(HitResults, Start, End, ObjectQueryParams);
		if (!bHit) continue;

		for (const FHitResult& Result : HitResults)
		{
			AKillingFloorLikeCharacter* HitPlayer = Cast<AKillingFloorLikeCharacter>(Result.GetActor());
			
			// 유효한 아군 캐릭터가 아니거나, 자기 자신을 쏘려고 하면 무시합니다.
			if (!HitPlayer || HitPlayer == Character)
			{
				continue;
			}

			// 현재까지 찾은 가장 가까운 캐릭터보다 더 가까운지 확인합니다.
			const float DistanceSq = (Result.ImpactPoint - Start).SizeSquared();
			if (DistanceSq < MinDistanceSq)
			{
				MinDistanceSq = DistanceSq;
				ClosestHitCharacter = HitPlayer;
				ClosestHitResult = Result;
			}
		}
	}

	// 피격 처리: 가장 가까운 캐릭터를 찾았다면 힐을 적용합니다.
	if (ClosestHitCharacter)
	{
		float Damage = IsSpecial ? WeaponData->special_damage : WeaponData->damage;
		
		UGameplayStatics::ApplyPointDamage(
			ClosestHitCharacter,
			Damage,
			Character->GetActorLocation(),
			ClosestHitResult,
			Character->GetInstigatorController(),
			GetOwner(),
			UHealDamageType::StaticClass()
		);
		
		UE_LOG(LogTemp, Log, TEXT("Heal: %s"), *ClosestHitCharacter->GetName());
		return true;
	}
	
	return false;
}