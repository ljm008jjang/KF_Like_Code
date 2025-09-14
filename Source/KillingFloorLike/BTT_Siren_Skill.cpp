// Fill out your copyright notice in the Description page of Project Settings.


#include "BTT_Siren_Skill.h"

#include "AIController.h"
#include "BaseCharacter.h"
#include "Monster.h"
#include "MonsterAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

EBTNodeResult::Type UBTT_Siren_Skill::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	SkillCooldown = 10.0f;
	Super::ExecuteTask(OwnerComp, NodeMemory);


	AMonsterAIController* AIController = Cast<AMonsterAIController>(OwnerComp.GetAIOwner());

	if (AIController == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	if (AIController->GetBlackboardComponent()->GetValueAsBool("IsSkillable") == false)
	{
		return EBTNodeResult::Failed;
	}

	AMonster* ControlledPawn = Cast<AMonster>(AIController->GetPawn());
	if (ControlledPawn == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	UAnimMontage* NewAnimMontage = ControlledPawn->GetAnimationMontage(EMonsterAnimationType::Skill, 0);

	FOnMontageEnded BlendOutDelegate;
	BlendOutDelegate.BindLambda([this, &OwnerComp,AIController](UAnimMontage* PlayedMontage, bool bInterrupted)
	{
		OnMontageEnded(PlayedMontage, bInterrupted, OwnerComp, AIController);
	});


	AIController->GetBlackboardComponent()->SetValueAsBool("IsSkillable", false);

	ControlledPawn->Multi_PlayCharacterAnim(NewAnimMontage);
	ControlledPawn->GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(BlendOutDelegate);

	return EBTNodeResult::InProgress;
}

void UBTT_Siren_Skill::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent& OwnerComp,
                                      AMonsterAIController* AIController)
{
	// 타이머 매니저 가져오기
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();

	// 혹시 이전에 설정된 타이머가 있다면 초기화합니다.
	TimerManager.ClearTimer(SkillTimerHandle);

	// 5초 후에 ResetSkillAvailability 함수를 호출하도록 타이머 설정
	// FTimerDelegate를 사용하여 파라미터를 넘겨줍니다.
	TWeakObjectPtr<AMonsterAIController> WeakController = AIController;
	
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindLambda([WeakController]()
	{
		if (AMonsterAIController* StrongController = WeakController.Get())
		{
			StrongController->GetBlackboardComponent()->SetValueAsBool("IsSkillable", true);
		}
	});

	//TODO 몬스터만의 쿨타임?
	TimerManager.SetTimer(SkillTimerHandle, TimerDelegate, SkillCooldown, false);

	if (bInterrupted)
	{
		FinishLatentAbort(OwnerComp);
	}

	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}
