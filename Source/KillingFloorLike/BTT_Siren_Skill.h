// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MonsterAIController.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTT_Siren_Skill.generated.h"

/**
 * 
 */
UCLASS()
class KILLINGFLOORLIKE_API UBTT_Siren_Skill : public UBTTaskNode
{
	GENERATED_BODY()

	EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory);
	
	void OnMontageEnded(UAnimMontage* Montage, bool bInterrupted, UBehaviorTreeComponent& OwnerComp,
	                    AMonsterAIController* AIController);

private:
	// 타이머를 관리하기 위한 핸들
	FTimerHandle SkillTimerHandle;
	UPROPERTY(EditAnywhere, Category = "Skill", meta = (ClampMin = "0.0", Units = "s"))
	float SkillCooldown = 10.0f;
};
