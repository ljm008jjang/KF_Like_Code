// Fill out your copyright notice in the Description page of Project Settings.


#include "DebugArrow.h"

#include "ShopManager.h"
#include "Components/SceneCaptureComponent2D.h"

// Sets default values
ADebugArrow::ADebugArrow()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = false;

	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCapture"));
	RootComponent = SceneCaptureComponent;
	ArrowMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArrowMesh"));
	ArrowMesh->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

// Called when the game starts or when spawned
void ADebugArrow::BeginPlay()
{
	Super::BeginPlay();
	
	OwningController = Cast<APlayerController>(GetOwner());
}

// Called every frame
void ADebugArrow::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 초기화가 되지 않았거나, 필수 객체들이 유효하지 않으면 아무것도 하지 않습니다.
	// 컨트롤러와 다른 필수 객체들이 유효한지 항상 확인합니다.
	if (IsValid(OwningController) == false || IsValid(ShopManager) == false || IsValid(ShopManager->GetCurrentShop()) == false)
	{
		return;
	}

	FRotator ViewportRotation;
	FVector ViewportLocation;
	
	// 플레이어의 시점을 컨트롤러에서 직접 가져옵니다.
	OwningController->GetPlayerViewPoint(ViewportLocation, ViewportRotation);

	// 시점 방향 기준으로 상점이 어디에 있는지를 계산하는 회전값
	FRotator Dir = (ShopManager->GetCurrentShop()->GetActorLocation() - ViewportLocation).GetSafeNormal().
		Rotation() - ViewportRotation;
	ArrowMesh->SetRelativeRotation(Dir);
}

void ADebugArrow::Init(AShopManager* NewShopManager)
{
	ShopManager = NewShopManager;
}
