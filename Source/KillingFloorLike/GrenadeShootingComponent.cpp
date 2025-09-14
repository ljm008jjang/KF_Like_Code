// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeShootingComponent.h"

#include "GrenadeWeapon.h"
#include "KFLikeGameInstance.h"
#include "KillingFloorLikeCharacter.h"
#include "ObjectPoolManager.h"

class UObjectPoolManager;
// Sets default values for this component's properties
UGrenadeShootingComponent::UGrenadeShootingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void UGrenadeShootingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
}

bool UGrenadeShootingComponent::Fire(AKillingFloorLikeCharacter* Character, FWeaponData* WeaponData, bool IsSpecial)
{
	if (Character == nullptr || WeaponData == nullptr)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (World == nullptr)
	{
		return false;
	}

	// 스폰할 수류탄 클래스가 설정되었는지 확인합니다.
	if (GrenadeClass == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("GrenadeClass is not set in %s! Please set it in the owning weapon's blueprint."),
		       *GetName());
		return false;
	}

	APlayerController* PlayerController = Cast<APlayerController>(Character->GetController());
	if (PlayerController == nullptr)
	{
		return false;
	}

	// 플레이어 시점(카메라)을 기준으로 발사 위치와 방향을 결정합니다.
	FVector SpawnLocation = GetComponentLocation();
	FRotator SpawnRotation = GetComponentRotation();

	// 스폰 파라미터 설정
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride =
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Character;


	// 오브젝트 풀에서 수류탄 가져오기
	UKFLikeGameInstance* GameInstance = GetWorld()->GetGameInstance<UKFLikeGameInstance>();
	if (GameInstance == nullptr) return false;

	UObjectPoolManager* PoolManager = GameInstance->GetSubsystem<UObjectPoolManager>();
	if (PoolManager == nullptr) return false;

	AGrenadeWeapon* GrenadeObject = PoolManager->GetFromPoolTemplate<AGrenadeWeapon>(
		GetWorld(), GrenadeClass, SpawnLocation, SpawnRotation,
		SpawnParams);

	if (GrenadeObject)
	{
		UStaticMeshComponent* GrenadeMesh = GrenadeObject->GetComponentByClass<UStaticMeshComponent>();
		if (GrenadeMesh)
		{
			// TODO: 발사 힘(5000.f)을 변수로 만들어 에디터에서 조절할 수 있게 하는 것이 좋습니다.
			GrenadeMesh->AddImpulse(Character->GetActorForwardVector() * GrenadeMesh->GetMass() * 500);
		}
		GrenadeObject->Server_SetDelayBoom();
	}


	return true;
}
