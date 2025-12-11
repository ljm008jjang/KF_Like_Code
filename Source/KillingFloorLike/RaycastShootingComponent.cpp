// Fill out your copyright notice in the Description page of Project Settings.


#include "RaycastShootingComponent.h"

#include "BaseWeapon.h"
#include "BaseCharacter.h"
#include "DebugManagerComponent.h"
#include "KillingFloorLikeCharacter.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
URaycastShootingComponent::URaycastShootingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void URaycastShootingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}


// Called every frame
void URaycastShootingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}


bool URaycastShootingComponent::Fire(AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial)
{
	FVector Start, End;
	if (!GetTraceStartEnd(Character, Start, End)) return false;

	UDebugManagerComponent::DrawDebugLineManager(GetWorld(), Start, End, FColor::Red, false, 5.0f);

	TArray<FHitResult> HitResults;
	FCollisionObjectQueryParams ObjectQueryParams;
	ObjectQueryParams.AddObjectTypesToQuery(ECC_PhysicsBody);
	ObjectQueryParams.AddObjectTypesToQuery(ECC_WorldStatic);

	const bool bHit = GetWorld()->LineTraceMultiByObjectType(HitResults, Start, End, ObjectQueryParams);
	if (!bHit) return true;

	TArray<FHitResult> FinalTargets;
	//타겟
	FilterAndPrioritizeHits(HitResults, FinalTargets);
	//데미지 적용
	ApplyDamageWithPenetration(FinalTargets, Character, WeaponData, IsSpecial);

	return true;
}


bool URaycastShootingComponent::GetTraceStartEnd(const AKillingFloorLikeCharacter* Character, FVector& OutStart, FVector& OutEnd) const
{
	if (IsValid(Character) == false || IsValid(Character->GetInstigatorController()) == false)
	{
		return false;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(Character->GetInstigatorController());
	if (IsValid(PlayerController) == false || IsValid(PlayerController->PlayerCameraManager) == false)
	{
		return false;
	}

	OutStart = PlayerController->PlayerCameraManager->GetCameraLocation();
	const FVector Direction = PlayerController->PlayerCameraManager->GetCameraRotation().Vector();
	// TODO: 사정거리는 WeaponData에서?
	constexpr float TraceDistance = 10000.0f;
	OutEnd = OutStart + Direction * TraceDistance;

	return true;
}

void URaycastShootingComponent::FilterAndPrioritizeHits(const TArray<FHitResult>& RawHits, TArray<FHitResult>& OutFinalTargets) const
{
	TMap<TWeakObjectPtr<ABaseCharacter>, FHitResult> BestHitPerCharacter;
	static const FName HeadBoneName = FName(TEXT("Head"));

	for (const FHitResult& Hit : RawHits)
	{
		ABaseCharacter* HitCharacter = Cast<ABaseCharacter>(Hit.GetActor());

		// [변경 2] IsValid로 명확하게 유효성 검사 (PendingKill 상태 등 체크)
		if (IsValid(HitCharacter))
		{
			// Key로 사용할 Weak Pointer 생성
			TWeakObjectPtr<ABaseCharacter> WeakCharacter = HitCharacter;

			FHitResult* ExistingHit = BestHitPerCharacter.Find(WeakCharacter);
          
			if (!ExistingHit)
			{
				// 첫 유효타 등록
				BestHitPerCharacter.Add(WeakCharacter, Hit);
			}
			// 이미 타격 정보가 있지만, 현재 타격이 헤드샷이고 기존 타격이 헤드샷이 아닐 경우 교체
			else if (Hit.BoneName == HeadBoneName && ExistingHit->BoneName != HeadBoneName)
			{
				// Key(WeakCharacter)를 통해 안전하게 접근하여 Value 업데이트
				BestHitPerCharacter[WeakCharacter] = Hit;
			}
		}
		else
		{
			// 캐릭터가 아닌 물체(벽 등)에 맞았으므로 관통 로직 종료
			break;
		}
	}

	if (BestHitPerCharacter.Num() > 0)
	{
		// TMap의 Value들(FHitResult)만 추출하여 배열로 변환
		// (GenerateValueArray는 내부적으로 유효한 Key-Value 쌍만 순회하므로 안전)
		BestHitPerCharacter.GenerateValueArray(OutFinalTargets);

		// 거리순 정렬
		OutFinalTargets.Sort([](const FHitResult& A, const FHitResult& B)
		{
		   return A.Distance < B.Distance;
		});
	}
}
void URaycastShootingComponent::ApplyDamageWithPenetration(const TArray<FHitResult>& Targets, AKillingFloorLikeCharacter* Character, const FWeaponData* WeaponData, bool IsSpecial) const
{
	int32 HitCount = 0;
	for (const FHitResult& HitInfo : Targets)
	{
		ABaseCharacter* HitTarget = Cast<ABaseCharacter>(HitInfo.GetActor());
		if (IsValid(HitTarget) == false || !Character->IsAttackableUnitType(HitTarget))
		{
			continue;
		}

		// 관통 시 데미지 감소 공식을 적용합니다.
		const float BaseDamage = IsSpecial ? WeaponData->special_damage : WeaponData->damage;
		const float Damage = BaseDamage * FMath::Pow(WeaponData->penetration_damage, HitCount);

		UGameplayStatics::ApplyPointDamage(
			HitTarget,
			Damage,
			Character->GetActorForwardVector(), // 총알의 방향
			HitInfo,
			Character->GetInstigatorController(),
			GetOwner(), // 데미지를 유발한 액터 (무기)
			UDamageType::StaticClass()
		);

		UE_LOG(LogTemp, Log, TEXT("Hit: %s on bone %s with %.2f damage."), *HitTarget->GetName(), *HitInfo.BoneName.ToString(), Damage);

		HitCount++;
		// 최대 관통 횟수에 도달하면 중지합니다.
		if (HitCount >= WeaponData->penetration)
		{
			break;
		}
	}
}
