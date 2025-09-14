// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeWeapon.h"

#include "ObjectPoolManager.h"
#include "RadialDamageType.h"
#include "ResourceManager.h"
#include "SoundManager.h"
#include "Kismet/GameplayStatics.h"

class UResourceManager;
class USoundManager;

void AGrenadeWeapon::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(BoomTimerHandle);
	Super::EndPlay(EndPlayReason);
}

bool AGrenadeWeapon::CheckFireWeapon(bool IsSpecial)
{
	if (LoadedAmmo <= 0)
	{
		return false;
	}

	bool result = Super::CheckFireWeapon(IsSpecial);

	if (result)
	{
		SetLoadedAmmo(LoadedAmmo - 1);
	}
	return result;
}

void AGrenadeWeapon::Server_SetDelayBoom_Implementation()
{
	GetWorldTimerManager().ClearTimer(BoomTimerHandle);

	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([this]()
	{
		this->Boom();
	});

	GetWorldTimerManager().SetTimer(BoomTimerHandle, TimerDelegate, 3.0f, false);
}

void AGrenadeWeapon::Boom()
{
	// 1. 폭발의 중심점은 수류탄 액터의 현재 위치입니다.
	const FVector ExplosionOrigin = GetActorLocation();

	// 2. 피해 계산에서 제외할 액터 목록을 만듭니다. (수류탄 자신, 던진 플레이어 등)
	TArray<AActor*> IgnoreActors;
	IgnoreActors.Add(this);
	if (GetOwner())
	{
		IgnoreActors.Add(GetOwner());
	}

	// 3. 범위 내 모든 대상에게 광역 피해를 적용합니다.
	UGameplayStatics::ApplyRadialDamage(
		this, // 월드 컨텍스트
		GetDamage(), // 기본 피해량
		ExplosionOrigin, // 폭발 원점
		500.0f, // 폭발 반경
		URadialDamageType::StaticClass(), // 피해 유형 (BaseWeapon에서 상속받은 프로퍼티 사용)
		IgnoreActors, // 무시할 액터 목록
		this, // 피해 유발자 (수류탄 자신)
		GetInstigatorController(), // 피해를 가한 컨트롤러
		false, // true: 거리에 상관없이 최대 피해, false: 거리에 따라 피해량 감소
		ECC_Visibility // 피해를 막는 벽 등을 확인하기 위한 콜리전 채널
	);

	USoundManager* SoundManager = USoundManager::GetSoundManager(this);
	// 3. 배경음 변경 (TODO: 향후 DB 또는 설정값으로 관리)
	SoundManager->Multi_Play3DSound(GetGameInstance()->GetSubsystem<UResourceManager>()->LoadSound(
		                                "/Game/KF/Weapon/Sound/Grenade/Nade_Explode_Cue"), GetActorLocation());
	
	GetGameInstance()->GetSubsystem<UObjectPoolManager>()->ReturnToPool(this);
}

FText AGrenadeWeapon::GetAmmoText()
{
	return FText::FromString(FString::FromInt(LoadedAmmo));
}

void AGrenadeWeapon::AddAmmo(int AddAmmoAmount)
{

	int32 FinalAmmo = LoadedAmmo + AddAmmoAmount;
	if (FinalAmmo > GetAmmoCapacity())
	{
		FinalAmmo = GetAmmoCapacity();
	}

	SetLoadedAmmo(FinalAmmo);
}

void AGrenadeWeapon::Server_BuyWeaponFill_Implementation()
{
	// 1) 서버 권한 및 유효성 검사
	if (!HasAuthority())
	{
		return;
	}
	// 무기 소유자 확인, 상점 이용 가능 여부 등 추가 검증
	// if (Weapon->GetOwner() != this || !bCanUseShop) return;



	// 3) 서버에서 비용 재계산(치트 방지)
	//    - 탄창 용량/가격/현재 돈/남은 탄약을 바탕으로
	//    - "탄창 단위" 구매 비용과 가능한 수량 계산
	const int32 MagCap = GetMagazineCapacity();
	const int32 MagCost = GetAmmoMagCost();
	const int32 MaxAmmo = GetAmmoCapacity();

	if (MagCap <= 0 || MagCost <= 0 || MaxAmmo <= 0)
	{
		return;
	}

	const int32 MyMoney = Character->GetMoney();
	const int32 Missing = FMath::Max(0, MaxAmmo - LoadedAmmo); // 접근자/세터는 실제 코드에 맞게
	if (Missing <= 0 || MyMoney < MagCost)
	{
		return; // 이미 가득이거나 최소 1탄창 비용도 없음
	}
	
	const int32 MagsAffordable = MyMoney / MagCost;
	const int32 MagsToBuy = FMath::Clamp(MagsAffordable, 0, Missing);
	if (MagsToBuy <= 0)
	{
		return;
	}

	const int32 Cost = MagsToBuy * MagCost;
	const int32 AmmoToAdd = MagsToBuy * MagCap;

	// 4) 돈 차감 → 탄약 추가(용량 클램프) → 복제/알림
	Character->SetMoney(Character->GetMoney() - Cost); // 서버에서 권위 있게 차감
	AddAmmo(AmmoToAdd); // 내부에서 용량 클램프 보장
}

FText AGrenadeWeapon::GetShopAmmoText()
{
	return FText::FromString(FString::FromInt(LoadedAmmo) + " / " + FString::FromInt(GetAmmoCapacity()));
}
