// Fill out your copyright notice in the Description page of Project Settings.


#include "MeleeShootingComponent.h"

#include "DebugManagerComponent.h"
#include "KillingFloorLikeCharacter.h"
#include "SoundManager.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UMeleeShootingComponent::UMeleeShootingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UMeleeShootingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UMeleeShootingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

bool UMeleeShootingComponent::Fire(AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial)
{
	if (IsValid(Character) == false){
		return false;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Character->GetInstigatorController());
	if (IsValid(PlayerController) == false)
	{
		return false;
	}

	// 1. 부채꼴 모양으로 레이캐스트를 실행하여 모든 잠재적 피격 결과를 수집합니다.
	TArray<FHitResult> RawHits;
	PerformFanTrace(PlayerController, RawHits);

	// 2. 캐릭터별로 하나의 유효타만 남기고, 헤드샷을 우선적으로 처리합니다.
	TMap<ABaseCharacter*, FHitResult> BestHits;
	FilterAndPrioritizeHits(RawHits, BestHits);

	// 3. 최종 타겟들에게 데미지를 적용합니다.
	const bool bHitConfirmed = ApplyMeleeDamage(BestHits, Character, WeaponData, IsSpecial);

	// 4. 공격 성공 여부에 따라 적절한 사운드를 재생합니다.
	PlayHitSound(bHitConfirmed, Character->GetActorLocation());

	return true;
}

/** 부채꼴 모양으로 레이캐스트를 실행하여 모든 잠재적 피격 결과를 수집합니다. */
void UMeleeShootingComponent::PerformFanTrace(const APlayerController* PlayerController, TArray<FHitResult>& OutRawHits) const
{
	// 참고: 현재 공격 기준점이 카메라입니다. 일반적인 근접 공격을 위해서는
	// Character->GetActorLocation()과 Character->GetActorForwardVector()를 사용하는 것을 고려해볼 수 있습니다.
	const FVector Start = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FRotator CameraRotation = PlayerController->PlayerCameraManager->GetCameraRotation();
	const FVector ForwardVec = CameraRotation.Vector();

	// UPROPERTY로 노출된 변수를 사용하여 유연성을 확보하고, 0으로 나누는 것을 방지합니다.
	const int32 Segments = FMath::Max(2, TraceSegments);
	const float Angle = AttackAngle;
	const float SegmentAngle = Angle / (Segments - 1);
	const float HalfAngle = Angle * 0.5f;

	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);

	for (int32 i = 0; i < Segments; ++i)
	{
		const float ThisAngle = HalfAngle - SegmentAngle * i;
		const FVector RotationAxis = CameraRotation.Quaternion().GetUpVector();
		const FQuat RotationQuat = FQuat(RotationAxis, FMath::DegreesToRadians(ThisAngle));

		const FVector End = Start + RotationQuat.RotateVector(ForwardVec) * AttackRange;

		UDebugManagerComponent::DrawDebugLineManager(GetWorld(), Start, End, FColor::Red, false, 5.0f);

		TArray<FHitResult> SegmentHits;
		if (GetWorld()->LineTraceMultiByObjectType(SegmentHits, Start, End, ObjectQueryParams))
		{
			OutRawHits.Append(SegmentHits);
		}
	}
}

/** 피격 결과 중 캐릭터별로 하나의 유효타만 남기고, 헤드샷을 우선적으로 처리합니다. */
void UMeleeShootingComponent::FilterAndPrioritizeHits(const TArray<FHitResult>& RawHits, TMap<ABaseCharacter*, FHitResult>& OutBestHitPerCharacter) const
{
	// FName을 사용하면 매번 FString으로 변환하는 것보다 훨씬 효율적입니다.
	static const FName HeadBoneName(TEXT("Head"));

	for (const FHitResult& Result : RawHits)
	{
		ABaseCharacter* HitCharacter = Cast<ABaseCharacter>(Result.GetActor());
		if (IsValid(HitCharacter) == false) continue;

		FHitResult* ExistingHit = OutBestHitPerCharacter.Find(HitCharacter);

		if (!ExistingHit)
		{
			// 이 캐릭터에 대한 첫 번째 유효타인 경우 추가합니다.
			OutBestHitPerCharacter.Add(HitCharacter, Result);
		}
		// 이미 몸을 맞췄는데, 더 뒤에 있는 헤드샷이 감지된 경우 헤드샷으로 교체합니다.
		else if (Result.BoneName.ToString().Contains(TEXT("Head")) && !ExistingHit->BoneName.ToString().Contains(TEXT("Head")))
		{
			// 참고: 뼈 이름이 "Head"로 정확히 일치한다면, Result.BoneName == HeadBoneName 비교가 더 빠릅니다.
			// 기존 기능 유지를 위해 Contains()를 사용합니다.
			OutBestHitPerCharacter[HitCharacter] = Result;
		}
	}
}

/** 최종 타겟 목록에 데미지를 적용하고, 실제 피격이 발생했는지 여부를 반환합니다. */
bool UMeleeShootingComponent::ApplyMeleeDamage(const TMap<ABaseCharacter*, FHitResult>& BestHits, AKillingFloorLikeCharacter* Character, const FWeaponData* WeaponData, bool IsSpecial) const
{
	bool bHitConfirmed = false;

	for (const auto& Pair : BestHits)
	{
		ABaseCharacter* Target = Pair.Key;
		const FHitResult& HitInfo = Pair.Value;

		UE_LOG(LogTemp, Warning, TEXT("%s : %s"), *Target->GetName(), *HitInfo.BoneName.ToString());

		if (Character && Character->IsAttackableUnitType(Target))
		{
			const float Damage = IsSpecial ? WeaponData->special_damage : WeaponData->damage;
			// 데미지 방향을 더 정확하게 계산하여 물리적 반응을 개선합니다.
			const FVector HitDirection = (Target->GetActorLocation() - Character->GetActorLocation()).GetSafeNormal();

			UGameplayStatics::ApplyPointDamage(
				Target, Damage, HitDirection, HitInfo,
				Character->GetInstigatorController(), GetOwner(), UDamageType::StaticClass()
			);

			bHitConfirmed = true;
			UE_LOG(LogTemp, Log, TEXT("Melee Hit: %s"), *Target->GetName());
		}
	}
	return bHitConfirmed;
}

/** 공격 성공 여부에 따라 적절한 사운드를 재생합니다. */
void UMeleeShootingComponent::PlayHitSound(bool bWasHit, const FVector& SoundLocation)
{
	// 매직 넘버 대신 의미 있는 상수를 사용하여 가독성을 높입니다.
	constexpr int32 HitSoundIndex = 2;
	constexpr int32 MissSoundIndex = 0;

	const EWeaponSoundType SoundType = EWeaponSoundType::Hit;
	const int32 SoundIndex = bWasHit ? HitSoundIndex : MissSoundIndex;

	if (USoundManager* SoundManager = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<USoundManager>())
	{
		if (USoundBase* SoundToPlay = GetSoundBase(SoundType, SoundIndex))
		{
			SoundManager->Multi_Play3DSound(SoundToPlay, SoundLocation);
		}
	}
}


USoundBase* UMeleeShootingComponent::GetSoundBase(EWeaponSoundType SoundType, int32 index)
{
	return SoundMap[SoundType].SoundBases[index];
}
