// Fill out your copyright notice in the Description page of Project Settings.


#include "ConsumableWeapon.h"

#include "PubSubManager.h"
#include "Net/UnrealNetwork.h"

void AConsumableWeapon::BeginPlay()
{
	Super::BeginPlay();
	SetLoadedAmmo(GetMagazineCapacity());
	SetSavedAmmo(GetMagazineCapacity());
}

void AConsumableWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	// CurrentTimeDilation 변수를 모든 클라이언트에게 복제합니다.
	DOREPLIFETIME(AConsumableWeapon, LoadedAmmo);
	DOREPLIFETIME(AConsumableWeapon, SavedAmmo);
}

void AConsumableWeapon::OnRep_Ammo()
{
	if (Character == nullptr)
	{
		return;
	}
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Player_AmmoChange, this,
	                                                           FGameplayMessage_PlayerAmmoChange(Character, this));
}


bool AConsumableWeapon::CheckFireWeapon(bool IsSpecial)
{
	if (LoadedAmmo <= 0)
	{
		return false;
	}

	bool result = Super::CheckFireWeapon(IsSpecial);
	int32 AmmoConsume = IsSpecial ? GetWeaponData().ammo_consume_special : GetWeaponData().ammo_consume;

	if (result)
	{
		SetLoadedAmmo(LoadedAmmo - AmmoConsume);
	}
	return result;
}

bool AConsumableWeapon::IsAttackable()
{
	return Super::IsAttackable() && GetLoadedAmmo() >= GetWeaponData().ammo_consume;
}

FText AConsumableWeapon::GetShopAmmoText()
{
	return FText::FromString(FString::FromInt(SavedAmmo) + " / " + FString::FromInt(GetAmmoCapacity()));
}


void AConsumableWeapon::AddAmmo(int AddAmmoAmount)
{
	int32 FinalAmmo = SavedAmmo + AddAmmoAmount;
	if (FinalAmmo > GetAmmoCapacity())
	{
		FinalAmmo = GetAmmoCapacity();
	}

	SetSavedAmmo(FinalAmmo);
}

int32 AConsumableWeapon::GetAmmoFillCost()
{
	// 방어적 체크
	if (Character == nullptr)
	{
		return 0;
	}

	const int32 MagCap = GetMagazineCapacity();
	const int32 MagCost = GetAmmoMagCost();
	const int32 MaxAmmo = GetAmmoCapacity();

	if (MagCap <= 0 || MagCost <= 0 || MaxAmmo <= 0)
	{
		return 0;
	}

	// 현재 부족한 탄약(저장 탄약 기준)
	const int32 MissingAmmo = FMath::Max(0, MaxAmmo - SavedAmmo);
	if (MissingAmmo <= 0)
	{
		return 0; // 이미 가득 참
	}

	// 필요한 "탄창 수"는 올림으로 계산
	const int32 MagsNeededToFull = FMath::CeilToInt(static_cast<float>(MissingAmmo) / static_cast<float>(MagCap));
	const int32 CostToFull = MagsNeededToFull * MagCost;

	// 지불 가능한 최대 금액(탄창 단위로 내림)
	const int32 Money = Character->GetMoney();
	const int32 Affordable = (Money / MagCost) * MagCost;

	return FMath::Min(CostToFull, Affordable);
}

void AConsumableWeapon::Server_BuyWeaponFill_Implementation()
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
	const int32 Missing = FMath::Max(0, MaxAmmo - GetSavedAmmo()); // 접근자/세터는 실제 코드에 맞게
	if (Missing <= 0 || MyMoney < MagCost)
	{
		return; // 이미 가득이거나 최소 1탄창 비용도 없음
	}

	// 필요한 탄창 수(올림), 살 수 있는 탄창 수(내림) → 실제 구매 탄창 수
	const int32 MagsNeeded = FMath::CeilToInt(static_cast<float>(Missing) / MagCap);
	const int32 MagsAffordable = MyMoney / MagCost;
	const int32 MagsToBuy = FMath::Clamp(MagsAffordable, 0, MagsNeeded);
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


FText AConsumableWeapon::GetAmmoText()
{
	return FText::AsNumber(LoadedAmmo); // + " / " + FString::FromInt(SavedAmmo);
}

FText AConsumableWeapon::GetClipText()
{
	return FText::AsNumber(ceil((float)SavedAmmo / GetMagazineCapacity()));
}


void AConsumableWeapon::Server_ReloadAmmoOne_Implementation()
{
	if (HasAuthority() == false)
	{
		return;
	}
	
	ReloadAmmoAdd(1);

	if (IsReloadable())
	{
		return;
	}
	
	Multi_ReloadAmmoOneCallback();
}

void AConsumableWeapon::Multi_ReloadAmmoOneCallback_Implementation()
{
	UAnimMontage* ReloadMontage = GetWeaponMontage(EWeaponAnimationType::Reload);
	Character->GetMesh1P()->GetAnimInstance()->Montage_JumpToSection(FName("ReloadAll"), ReloadMontage);
	UAnimMontage* PlayerReloadMontage = GetPlayerAnimation(EWeaponAnimationType::Reload);
	Character->GetMesh1P()->GetAnimInstance()->Montage_Stop(0.0f, PlayerReloadMontage);
}

void AConsumableWeapon::ReloadAmmoAdd(int ReloadedAmmo)
{
	if (HasAuthority() == false)
	{
		return;
	}
	if (SavedAmmo <= 0)
	{
		return;
	}

	SetLoadedAmmo(LoadedAmmo + ReloadedAmmo);
	SetSavedAmmo(SavedAmmo - ReloadedAmmo);
}

bool AConsumableWeapon::IsReloadable()
{
	if (LoadedAmmo >= GetMagazineCapacity())
	{
		return false;
	}

	if (SavedAmmo <= 0)
	{
		return false;
	}

	if (AnimationMap.Contains(EWeaponAnimationType::Reload) == false || AnimationMap[EWeaponAnimationType::Reload].
	                                                                    Montages.Max() <= 0)
	{
		return false;
	}

	return true;
}
