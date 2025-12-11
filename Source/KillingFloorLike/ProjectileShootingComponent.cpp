// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileShootingComponent.h"

#include "DebugManagerComponent.h"
#include "BaseWeapon.h"
#include "KillingFloorLikeCharacter.h"
#include "KillingFloorLikeProjectile.h"
#include "ObjectPoolManager.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ICookInfo.h"

class AKillingFloorLikeCharacter;
// Sets default values for this component's properties
UProjectileShootingComponent::UProjectileShootingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UProjectileShootingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

bool UProjectileShootingComponent::Fire(AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial)
{
	// 1. 유효성 검사
	UWorld* World = GetWorld();
	if (IsValid(Character) == false || !WeaponData || IsValid(ProjectileClass) == false || IsValid(World) == false)
	{
		return false;
	}

	// 2. 발사 시작 위치 및 방향 계산
	FVector SpawnLocation;
	FRotator SpawnRotation;
	if (!GetFireStartProperties(Character, SpawnLocation, SpawnRotation))
	{
		return false;
	}

	// 3. 스폰 파라미터 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButDontSpawnIfColliding;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Character->GetInstigator();

	// 4. 발사할 펠릿(탄환) 수와 데미지 계산
	const int32 PelletCount = WeaponData->pellets;
	const float Damage = IsSpecial ? WeaponData->special_damage : WeaponData->damage;

	// 5. 각 펠릿에 대해 투사체 생성 및 초기화
	for (int32 i = 0; i < PelletCount; i++)
	{
		SpawnAndInitializeProjectile(WeaponData, SpawnLocation, SpawnRotation, SpawnParams, Damage, World);
	}

	return true;
}

bool UProjectileShootingComponent::GetFireStartProperties(const AKillingFloorLikeCharacter* Character, FVector& OutLocation, FRotator& OutRotation) const
{
	if (IsValid(Character) == false) return false;

	const APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	if (IsValid(PlayerController) == false) return false;

	// 1. 발사 방향은 항상 카메라(플레이어 시점)를 기준으로 합니다.
	PlayerController->GetPlayerViewPoint(OutLocation, OutRotation);

	OutLocation += OutRotation.RotateVector(FVector(100, 0, 0)); // Muzzle 위치 보정

	// 2. 발사 위치는 무기의 총구(Muzzle) 소켓에서 가져옵니다.
	// 이는 카메라가 아닌 실제 총구에서 발사되도록 하여 정확도를 높입니다.
	/*if (const ABaseWeapon* Weapon = Cast<ABaseWeapon>(GetOwner()))
	{
		if (const USceneComponent* WeaponMesh = Character->GetMesh1P())
		{
			// FName은 생성 비용이 있으므로 static으로 선언하여 한 번만 생성합니다.
			static const FName MuzzleSocketName = FName("tip");
			OutLocation = WeaponMesh->GetSocketLocation(MuzzleSocketName);
		}
	}*/
	// 만약 소켓 위치를 가져오지 못했다면, 카메라 위치를 그대로 사용합니다.

	return true;
}

void UProjectileShootingComponent::SpawnAndInitializeProjectile(
	const FWeaponData* WeaponData,
	const FVector& SpawnLocation,
	const FRotator& BaseRotation,
	const FActorSpawnParameters& SpawnParams,
	float Damage,
	UWorld* World) const
{
	// 탄 퍼짐(Spread) 각도를 계산합니다.
	// 데이터에 있는 spread 값의 의미를 명확히 하기 위해 상수를 사용합니다.
	constexpr float SpreadToAngleMultiplier = 0.1f;
	const float SpreadAngle = WeaponData->spread * SpreadToAngleMultiplier;

	// 퍼짐을 적용한 최종 발사 각도를 계산합니다.
	FRotator AdjustedRotation = BaseRotation;
	if (SpreadAngle > 0.f)
	{
		AdjustedRotation.Yaw += FMath::RandRange(-SpreadAngle, SpreadAngle);
		AdjustedRotation.Pitch += FMath::RandRange(-SpreadAngle, SpreadAngle);
	}

	// 디버그 라인을 그립니다.
	constexpr float DebugLineDistance = 1000.0f;
	UDebugManagerComponent::DrawDebugLineManager(
		World, SpawnLocation, SpawnLocation + AdjustedRotation.Vector() * DebugLineDistance,
		FColor::Red, false, 5.0f
	);

	// 오브젝트 풀에서 투사체를 가져옵니다.
	UObjectPoolManager* PoolManager = UGameplayStatics::GetGameInstance(World)->GetSubsystem<UObjectPoolManager>();
	if (IsValid(PoolManager) == false) return;

	AKillingFloorLikeProjectile* Projectile = PoolManager->GetFromPoolTemplate<AKillingFloorLikeProjectile>(
		World, ProjectileClass, SpawnLocation, AdjustedRotation, SpawnParams);

	if (IsValid(Projectile))
	{
		// 투사체를 초기화합니다.
		Projectile->Initialize(
			GetOwner(), Cast<APawn>(SpawnParams.Instigator),
			Damage, WeaponData->penetration, WeaponData->penetration_damage
		);
	}
}

