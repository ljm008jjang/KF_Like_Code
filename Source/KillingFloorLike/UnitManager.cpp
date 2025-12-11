// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitManager.h"

#include "Monster.h"
#include "ObjectPoolManager.h"
#include "PubSubManager.h"
#include "SpawnPoint.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

class AMonster;
// Sets default values
AUnitManager::AUnitManager()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
}

void AUnitManager::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	// MaxSpawnedMonsters 변수가 서버에서 클라이언트로 복제되도록 설정합니다.
	DOREPLIFETIME(AUnitManager, MaxSpawnedMonsters);
	DOREPLIFETIME(AUnitManager, RemainMonsterCount);
}

// Called when the game starts or when spawned
void AUnitManager::BeginPlay()
{
	Super::BeginPlay();

	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ASpawnPoint::StaticClass(), SpawnPoints);
}

// Called every frame
void AUnitManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AUnitManager::OnRep_RemainMonsterCount()
{
	GetGameInstance()->GetSubsystem<UPubSubManager>()->Publish(EPubSubTag::Mode_MonsterCountChange, this,
	                                                           FGameplayMessage_MonsterCount(this, RemainMonsterCount));
}

void AUnitManager::RefillMonsterPool(const int32 CurrentWave)
{
	if (HasAuthority() == false)
	{
		return;
	}
	// Adjust Monster Counts Based on Current Wave
	SetMaxSpawnedMonsters(0);
	MonsterPool.Add(EMonsterType::Clot, 0);
	MonsterPool.Add(EMonsterType::Gorefast, 0);
	MonsterPool.Add(EMonsterType::Stalker, 0);
	MonsterPool.Add(EMonsterType::Crawler, 0);
	MonsterPool.Add(EMonsterType::Husk, 0);
	MonsterPool.Add(EMonsterType::Scrake, 0);
	MonsterPool.Add(EMonsterType::Fleshpound, 0);
	MonsterPool.Add(EMonsterType::Siren, 0);
	MonsterPool.Add(EMonsterType::Bloat, 0);

	//MonsterPool[EMonsterType::Husk] = 1;
	/*MonsterPool[EMonsterType::Gorefast] = 1;
	MonsterPool[EMonsterType::Stalker] = 1;
	MonsterPool[EMonsterType::Crawler] = 1;
	MonsterPool[EMonsterType::Husk] = 1;
	MonsterPool[EMonsterType::Scrake] = 1;
	MonsterPool[EMonsterType::Fleshpound] = 1;
	MonsterPool[EMonsterType::Siren] = 1;
	MonsterPool[EMonsterType::Bloat] = 1;*/

	//MonsterPool[EMonsterType::Husk] = 1;
	MonsterPool[EMonsterType::Clot] = 10 + FMath::CeilToInt(CurrentWave * 1.5f);
	MonsterPool[EMonsterType::Gorefast] = 5 + FMath::CeilToInt(CurrentWave * 1.3f);
	MonsterPool[EMonsterType::Crawler] = 5 + FMath::CeilToInt(CurrentWave * 1.2f);
	MonsterPool[EMonsterType::Stalker] = 5 + FMath::CeilToInt(CurrentWave * 1.0f);

	MonsterPool[EMonsterType::Husk] = 1;
	MonsterPool[EMonsterType::Scrake] = 1;
	MonsterPool[EMonsterType::Fleshpound] = 1;
	MonsterPool[EMonsterType::Bloat] = 1;
	MonsterPool[EMonsterType::Siren] = 1;


	// 중간 몬스터: 웨이브 중반부터 등장
	/*MonsterPool[EMonsterType::Bloat] = FMath::CeilToInt(CurrentWave * (CurrentWave >= 3 ? 0.9f : 0.0f));
	if (CurrentWave >= 4)
	{
		float SirenMultiplier = 0.4f + (CurrentWave - 4) * 0.05f; // 4웨이브 이후부터 서서히 증가
		MonsterPool.Add(EMonsterType::Siren, FMath::CeilToInt(CurrentWave * SirenMultiplier));
	}
	if (CurrentWave >= 5)
	{
		float ScrakeMultiplier = 0.5f + (CurrentWave - 5) * 0.07f;
		MonsterPool.Add(EMonsterType::Scrake, FMath::CeilToInt(CurrentWave * ScrakeMultiplier));
	}
	if (CurrentWave >= 6)
	{
		float HuskMultiplier = 0.4f + (CurrentWave - 6) * 0.05f;
		MonsterPool.Add(EMonsterType::Husk, FMath::CeilToInt(CurrentWave * HuskMultiplier));
	}
	if (CurrentWave >= 7)
	{
		float FleshpoundMultiplier = 0.3f + (CurrentWave - 7) * 0.1f;
		MonsterPool.Add(EMonsterType::Fleshpound, FMath::CeilToInt(CurrentWave * FleshpoundMultiplier));
	}*/

	for (const auto& Pair : MonsterPool)
	{
		int32 MonsterCount = Pair.Value;
		SetMaxSpawnedMonsters(MaxSpawnedMonsters + MonsterCount);
	}
	SetRemainMonsterCount(MaxSpawnedMonsters);
}

int32 AUnitManager::GetMaxSpawnedMonsters()
{
	return MaxSpawnedMonsters;
}

void AUnitManager::SpawnMonster()
{
	if (HasAuthority() == false)
	{
		return;
	}
	if (MonsterPool.Num() == 0)
	{
		return;
	}

	// 1. 스폰할 몬스터의 총 개수를 가중치로 사용하기 위해 계산합니다.
	int32 TotalMonsterCount = 0;
	for (const auto& Pair : MonsterPool)
	{
		TotalMonsterCount += Pair.Value;
	}

	// 스폰할 몬스터가 더 이상 없으면 함수를 종료합니다.
	if (TotalMonsterCount <= 0)
	{
		MonsterPool.Empty(); // 맵을 깨끗하게 비웁니다.
		return;
	}

	// 2. 0부터 (총 몬스터 수 - 1) 사이의 랜덤 숫자를 선택합니다.
	int32 RandomNumber = FMath::RandRange(0, TotalMonsterCount - 1);

	// 3. 가중치에 따라 스폰할 몬스터를 결정합니다.
	EMonsterType SelectedMonster = EMonsterType::None;
	for (const auto& Pair : MonsterPool)
	{
		if (RandomNumber < Pair.Value)
		{
			SelectedMonster = Pair.Key;
			break;
		}
		RandomNumber -= Pair.Value;
	}

	// 유효한 몬스터를 선택하지 못했다면 (예: 맵이 비어있는 등 예외 상황) 함수를 종료합니다.
	if (SelectedMonster == EMonsterType::None)
	{
		return;
	}

	// 4. 선택된 몬스터를 스폰하고, 풀에서 카운트를 1 감소시킵니다.
	SpawnMonster(SelectedMonster);
	MonsterPool[SelectedMonster]--;

	// 선택 사항: 만약 카운트가 0이 된 몬스터를 풀에서 제거하고 싶다면 아래 코드를 추가할 수 있습니다.
	// 이는 다음 번 가중치 계산 시 성능에 미미한 이점을 줄 수 있습니다.
	if (MonsterPool[SelectedMonster] <= 0)
	{
		MonsterPool.Remove(SelectedMonster);
	}
}

void AUnitManager::SpawnMonster(EMonsterType SelectedMonster)
{
	AActor* SpawnPoint = SpawnPoints[FMath::RandHelper(SpawnPoints.Num())];

	FActorSpawnParameters params;
	params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	AMonster* SpawnAnimal = GetGameInstance()->GetSubsystem<UObjectPoolManager>()->GetFromPoolTemplate<AMonster>(
		GetWorld(), MonsterClass[SelectedMonster],
		SpawnPoint->GetActorLocation(),
		SpawnPoint->GetActorRotation(), params);

	if (IsValid(SpawnAnimal) == false)
	{
		return;
	}

	SpawnAnimal->SpawnDefaultController();
	Monsters.Add(SpawnAnimal);
}


//TODO 매 프레임 체크해야하는 것 같은데?
int AUnitManager::GetSpawnedMonsterCount(bool IsAlive)
{
	if (HasAuthority() == false)
	{
		return 0;
	}
	int Result = 0;

	if (IsAlive)
	{
		for (AMonster* Monster : Monsters)
		{
			if (Monster->GetCurrentUnitState() != EUnitState::Dead)
			{
				Result++;
			}
		}
	}
	else
	{
		for (AMonster* Monster : Monsters)
		{
			if (Monster->GetCurrentUnitState() == EUnitState::Dead)
			{
				Result++;
			}
		}
	}

	return Result;
}

void AUnitManager::ClearUnitDB()
{
	if (HasAuthority() == false)
	{
		return;
	}
	Monsters.Empty();
}

TArray<AActor*> AUnitManager::GetSpawnPoints()
{
	return SpawnPoints;
}
