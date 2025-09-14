// Fill out your copyright notice in the Description page of Project Settings.


#include "SplineActor.h"

#include "KillingFloorLikeGameMode.h"
#include "KillingFloorLikeGameState.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraComponent.h"
#include "Components/SplineComponent.h"
#include "ShopManager.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
 ASplineActor::ASplineActor()
 {
	 // Tick 함수는 성능을 위해 비활성화하고, 주기적인 업데이트는 Timer를 사용합니다.
 	PrimaryActorTick.bCanEverTick = false;
 
 	SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
 	NiagaraEffect = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
 }
void ASplineActor::BeginPlay()
// Hidden Lines
 {
 	Super::BeginPlay();
 
 	// GameState는 자주 사용되므로 BeginPlay에서 한 번만 캐싱합니다.
 	CachedGameState = Cast<AKillingFloorLikeGameState>(UGameplayStatics::GetGameState(GetWorld()));
 	
 	GetWorld()->GetTimerManager().SetTimer(MoveTimerHandle, this, &ASplineActor::CreatePathTo, 1, true);
 }

void ASplineActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
 	GetWorld()->GetTimerManager().ClearTimer(MoveTimerHandle);
	Super::EndPlay(EndPlayReason);
}

void ASplineActor::CreatePathTo()
 {
 	// 1. 상태 검사: GameState가 유효하지 않거나, 현재 모드가 Break가 아니라면 이펙트를 끄고 종료합니다.
 	if (!IsValid(CachedGameState) || CachedGameState->GetCurrentModeType() != EModeType::Break)
 	{
 		NiagaraEffect->Deactivate();
 		return;
 	}
 
 	// Break 모드이므로 이펙트를 활성화합니다.
 	NiagaraEffect->Activate();
 
 	// 2. 필요 객체 확인: 경로 탐색에 필요한 플레이어 캐릭터와 상점 액터가 유효한지 확인합니다.
 	// 캐릭터가 아직 캐싱되지 않았다면 로컬 플레이어 캐릭터로 설정합니다.
 	if (!IsValid(Character))
 	{
 		Character = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
 	}
 
 	// 캐릭터가 여전히 유효하지 않으면 경로를 생성할 수 없습니다.
 	if (!IsValid(Character))
 	{
 		NiagaraEffect->Deactivate();
 		return;
 	}
 
 	const AShopManager* ShopManager = CachedGameState->GetShopManager();
 	const AActor* Shop = ShopManager ? ShopManager->GetCurrentShop() : nullptr;
 
 	// 상점이 유효하지 않으면 경로를 생성할 수 없습니다.
 	if (!IsValid(Shop))
 	{
 		NiagaraEffect->Deactivate();
 		return;
 	}
 
 	// 3. 경로 탐색: 네비게이션 시스템을 사용하여 캐릭터에서 상점까지의 경로를 찾습니다.
 	const FVector StartLocation = Character->GetActorLocation();
 	const FVector GoalLocation = Shop->GetActorLocation();
 	
 	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
 	if (!NavSys)
 	{
 		NiagaraEffect->Deactivate();
 		return;
 	}
 	
 	const UNavigationPath* Path = NavSys->FindPathToLocationSynchronously(GetWorld(), StartLocation, GoalLocation);
 	if (!Path || !Path->IsValid() || Path->PathPoints.Num() == 0)
 	{
 		NiagaraEffect->Deactivate();
 		return;
 	}
 
 	// 4. 스플라인 생성: 찾은 경로를 바탕으로 스플라인을 다시 만듭니다.
 	// 스플라인 액터 자체의 위치를 경로의 시작점으로 이동시킵니다.
 	SetActorLocation(StartLocation);
 	
 	SplineComponent->ClearSplinePoints();
 	for (const FVector& Point : Path->PathPoints)
 	{
 		SplineComponent->AddSplinePoint(Point, ESplineCoordinateSpace::World);
 	}
 	SplineComponent->UpdateSpline();
 }
