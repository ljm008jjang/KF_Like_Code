// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Monster_Skill.h"

#include "AIController.h"
#include "BaseCharacter.h"
#include "Monster.h"
#include "MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTT_Monster_Skill::UBTT_Monster_Skill()
{
	// 노드 이름을 설정하여 비헤이비어 트리 에디터에서 쉽게 식별할 수 있도록 합니다.
	NodeName = TEXT("Use Monster Skill");

	// Latent Task는 Tick이 필요하지 않으므로 비활성화하여 성능을 최적화합니다.
	bNotifyTick = false;
}

EBTNodeResult::Type UBTT_Monster_Skill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	SkillCooldown = 10.0f;
	Super::ExecuteTask(OwnerComp, NodeMemory);

	// 1. 유효성 검사: 필요한 컴포넌트와 액터들이 유효한지 확인합니다.
	AMonsterAIController* AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (AIController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	if (BlackboardComp == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// 블랙보드 키가 유효하게 설정되었는지 확인합니다.
	if (IsSkillableKey.SelectedKeyName.IsNone() || IsMoveableKey.SelectedKeyName.IsNone())
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTT_Monster_Skill: Blackboard keys are not set!"));
		return EBTNodeResult::Failed;
	}

	// 스킬 사용 가능 상태인지 확인합니다.
	if (BlackboardComp->GetValueAsBool(IsSkillableKey.SelectedKeyName) == false)
	{
		return EBTNodeResult::Failed;
	}

	AMonster* ControlledPawn = Cast<AMonster>(AIController->GetPawn());
	if (ControlledPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// 2. 애니메이션 몽타주 설정
	UAnimMontage* SkillMontage = ControlledPawn->GetAnimationMontage(EMonsterAnimationType::Skill, 0);
	if (SkillMontage == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("UBTT_Monster_Skill: Skill montage is not found on monster %s."), *GetNameSafe(ControlledPawn));
		return EBTNodeResult::Failed;
	}

	// 3. 몽타주 종료 시 호출될 델리게이트를 안전하게 바인딩합니다.
	// TWeakObjectPtr를 사용하여, 몽타주가 끝나기 전에 컨트롤러가 파괴되는 경우를 안전하게 처리합니다.
	TWeakObjectPtr<AMonsterAIController> AIControllerPtr = AIController;
	FOnMontageEnded BlendOutDelegate;
	BlendOutDelegate.BindUObject(this, &UBTT_Monster_Skill::OnMontageEnded, &OwnerComp, AIControllerPtr);

	// 4. AI 상태 변경 및 애니메이션 재생
	BlackboardComp->SetValueAsBool(IsMoveableKey.SelectedKeyName, false);
	BlackboardComp->SetValueAsBool(IsSkillableKey.SelectedKeyName, false);
	AIController->SetIsRotate(true);

	ControlledPawn->Multi_PlayCharacterAnim(SkillMontage);
	ControlledPawn->GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(BlendOutDelegate, SkillMontage);

	// 5. 태스크가 몽타주가 끝날 때까지 대기하도록 InProgress 상태를 반환합니다.
	return EBTNodeResult::InProgress;
}

void UBTT_Monster_Skill::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp,
                                        TWeakObjectPtr<AMonsterAIController> AIControllerPtr)
{
	// 람다나 타이머에 의해 호출될 때 AIController가 이미 파괴되었을 수 있으므로, 항상 유효성을 먼저 검사합니다.
	AMonsterAIController* AIController = AIControllerPtr.Get();
	if (!AIController || !OwnerComp)
	{
		// OwnerComp가 유효하지 않으면 태스크를 종료할 수 없으므로, 그냥 반환합니다.
		return;
	}

	// 몽타주가 중단된 경우, 상태를 원복하고 태스크를 '중단'으로 즉시 종료합니다.
	if (bInterrupted)
	{
		CleanupTask(*OwnerComp, AIController, true);
		return;
	}

	// 몽타주가 성공적으로 끝난 경우, 쿨다운 타이머를 설정합니다.
	if (UWorld* World = GetWorld())
	{
		// 혹시 이전에 설정된 타이머가 있다면 초기화합니다.
		World->GetTimerManager().ClearTimer(SkillTimerHandle);
		
		FTimerDelegate TimerDelegate;
		TimerDelegate.BindLambda([AIControllerPtr, SkillableKeyName = IsSkillableKey.SelectedKeyName]()
		{
			if (AMonsterAIController* StrongController = AIControllerPtr.Get())
			{
				if (UBlackboardComponent* BlackboardComp = StrongController->GetBlackboardComponent())
				{
					BlackboardComp->SetValueAsBool(SkillableKeyName, true);
				}
			}
		});

		World->GetTimerManager().SetTimer(SkillTimerHandle, TimerDelegate, SkillCooldown, false);
	}

	CleanupTask(*OwnerComp, AIController, false);
}

void UBTT_Monster_Skill::CleanupTask(UBehaviorTreeComponent& OwnerComp, AMonsterAIController* AIController, bool bIsInterrupted)
{
	// AI의 상태를 원래대로 복원합니다.
	if (UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent())
	{
		BlackboardComp->SetValueAsBool(IsMoveableKey.SelectedKeyName, true);
	}
	AIController->SetIsRotate(false);

	// 태스크를 종료합니다.
	if (bIsInterrupted)
	{
		FinishLatentAbort(OwnerComp);
	}
	else
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
}
