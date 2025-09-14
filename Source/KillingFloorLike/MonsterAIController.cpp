// Fill out your copyright notice in the Description page of Project Settings.


#include "MonsterAIController.h"

#include "KillingFloorLikeCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

/*AMonsterAIController::AMonsterAIController()
{
	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SetPerceptionComponent(*AIPerception);

	// Sight Config
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	SightConfig->SightRadius = 800.f;
	SightConfig->LoseSightRadius = 1200.f; 
	SightConfig->PeripheralVisionAngleDegrees = 60.f; 
	SightConfig->SetMaxAge(5.f); 
	SightConfig->AutoSuccessRangeFromLastSeenLocation = -1.f;

	// Detect only specific actors 
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;

	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());
    
	HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	HearingConfig->HearingRange = 500.f;
	HearingConfig->SetMaxAge(3.f);

	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerception->ConfigureSense(*HearingConfig);
	// ...
    
	AIPerception->OnPerceptionUpdated.AddDynamic(this, &ABaseAIController::PerceptionUpdated);
}*/

void AMonsterAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	SetActorRotation(DeltaSeconds);
}

/*void AMonsterAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	UBehaviorTreeComponent* BehaviorTree = InPawn->GetComponentByClass<UBehaviorTreeComponent>();
	if (BehaviorTree == nullptr)
	{
		return;
	}

	RunBehaviorTree(BehaviorTree->DefaultBehaviorTreeAsset());
}*/

void AMonsterAIController::SetActorRotation(float DeltaSeconds)
{
	if (Blackboard == nullptr)
	{
		return;
	}
	if (GetIsRotate() == false)
	{
		return;
	}

	AKillingFloorLikeCharacter* Target = Cast<
		AKillingFloorLikeCharacter>(Blackboard->GetValueAsObject(FName("Player")));
	if (Target == nullptr)
	{
		return;
	}

	FVector ActorLocation = GetPawn()->GetActorLocation();

	FVector TargetLocation = Target->GetActorLocation();

	FRotator LookRotation = UKismetMathLibrary::FindLookAtRotation(ActorLocation, TargetLocation);

	// 참고: 컨트롤러의 현재 회전값을 가져오는 것이 더 정확할 수 있습니다.
	FRotator CurrentControlRotation = GetControlRotation();
	FRotator TargetControlRotation = FMath::RInterpTo(CurrentControlRotation, LookRotation, DeltaSeconds, 20.f);

	FRotator ResultRotation = FRotator(0.f, TargetControlRotation.Yaw, 0.f);

	GetPawn()->SetActorRotation(ResultRotation);
}

void AMonsterAIController::OnDead()
{
	Destroy();
}
