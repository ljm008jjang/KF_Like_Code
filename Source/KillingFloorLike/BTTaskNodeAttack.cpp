// Fill out your copyright notice in the Description page of Project Settings.


#include "BTTaskNodeAttack.h"

#include "Monster.h"
#include "MonsterAIController.h"

void UBTTaskNodeAttack::PlayAttackMontage(AMonster* Monster, UAnimMontage* Montage)
{
	
	FOnMontageEnded OnMontageEnded;
	OnMontageEnded.BindLambda([this, Monster](UAnimMontage* Montage, bool bInterrupted)
	{
		FinishExecute(!bInterrupted);
		if (AMonsterAIController* AIController = Cast<AMonsterAIController>(Monster->GetController()))
		{
			AIController->SetIsRotate(false);
		}
	});
	
	Monster->Multi_PlayCharacterAnim(Montage);
	if (Monster->HasAuthority())
	{
		Monster->GetMesh()->GetAnimInstance()->Montage_SetEndDelegate(OnMontageEnded);
	}

	Monster->Multi_PlaySoundBase(EMonsterSoundType::Attack);
}
