// Fill out your copyright notice in the Description page of Project Settings.


#include "UnitIsAlive.h"

#include "KillingFloorLikeCharacter.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"

UUnitIsAlive::UUnitIsAlive(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	// 테스트의 기본 목적을 '필터링'으로 설정합니다.
	// 이렇게 하면 테스트에 실패한 아이템은 결과에서 즉시 제외됩니다.
	// Set the default purpose of the test to 'filtering'.
	// This will immediately exclude items that fail the test from the results.
	TestPurpose = EEnvTestPurpose::Filter;

	// 필터링 방식은 'Bool' (통과/실패) 방식임을 명시합니다.
	FilterType = EEnvTestFilterType::Match;

	// 점수 계산 방식이 아닌, 통과/실패 방식의 필터임을 명시합니다.
	// Specify that this is a pass/fail filter, not a scoring method.
	ValidItemType = UEnvQueryItemType_Actor::StaticClass();
}

void UUnitIsAlive::RunTest(FEnvQueryInstance& QueryInstance) const
{
	//Super::RunTest(QueryInstance);

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		// 쿼리 아이템에서 액터를 가져옵니다.
		// Get the actor from the query item.
		AActor* ItemActor = GetItemActor(QueryInstance, It.GetIndex());
		if (IsValid(ItemActor) == false)
		{
			continue; // 액터가 없으면 다음으로 넘어갑니다.
			// If there is no actor, move on to the next one.
		}

		// 가져온 액터를 실제 유닛 클래스로 형변환합니다.
		// Cast the retrieved actor to the actual unit class.
		AKillingFloorLikeCharacter* Unit = Cast<AKillingFloorLikeCharacter>(ItemActor);
		bool bIsAlive = false;

		if (Unit)
		{
			// 유닛의 생존 여부를 확인하는 로직입니다.
			// 여러분의 프로젝트에 맞게 이 부분을 수정하세요 (예: Health > 0).
			// This is the logic to check if the unit is alive.
			// Modify this part to fit your project (e.g., Health > 0).
			bIsAlive = Unit->IsDead() == true;
		}

		// SetItemScore의 마지막 파라미터가 테스트의 통과(true)/실패(false)를 결정합니다.
		// The last parameter of SetItemScore determines the pass (true) / fail (false) of the test.
		It.SetScore(TestPurpose, FilterType, bIsAlive, false);
	}
}
