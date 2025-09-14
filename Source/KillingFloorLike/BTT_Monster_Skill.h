// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MonsterAIController.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Monster_Skill.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UBTT_Monster_Skill : public UBTTaskNode
{
	GENERATED_BODY()
	UBTT_Monster_Skill();

	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	/** Called when the skill animation montage ends. */
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent* OwnerComp, TWeakObjectPtr<AMonsterAIController> AIControllerPtr);
 
	/** Resets AI state and finishes the latent task. */
	void CleanupTask(UBehaviorTreeComponent& OwnerComp, AMonsterAIController* AIController, bool bIsInterrupted);
 
	/** Blackboard key for the boolean value indicating if the AI can move. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsMoveableKey;
 
	/** Blackboard key for the boolean value indicating if the AI can use a skill. */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsSkillableKey;

	// 타이머를 관리하기 위한 핸들
	FTimerHandle SkillTimerHandle;
	
	UPROPERTY(EditAnywhere, Category = "Skill", meta = (ClampMin = "0.0", Units = "s"))
	float SkillCooldown = 10.0f;
};
