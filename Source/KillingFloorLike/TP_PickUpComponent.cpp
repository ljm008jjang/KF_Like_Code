// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_PickUpComponent.h"

#include "BaseWeapon.h"

UTP_PickUpComponent::UTP_PickUpComponent()
{
	// Setup the Sphere Collision
	SphereRadius = 32.f;
}


void UTP_PickUpComponent::BeginPlay()
{
	Super::BeginPlay();

	// Register our Overlap Event
	if (GetOwner() == nullptr)
	{
		return;
	}
	AActor* Actor = Cast<AActor>(GetOwner());
	//Character가 있다는건 어떤 캐릭터에게 소유되었음. 설정 필요 X
	if (Actor != nullptr && Actor->HasAuthority())
	{
		OnComponentBeginOverlap.AddDynamic(this, &UTP_PickUpComponent::OnSphereBeginOverlap);
		//OnDrop.AddDynamic(this, &UTP_PickUpComponent::AfterDrop);
	}
}

void UTP_PickUpComponent::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                               UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                               const FHitResult& SweepResult)
{
	// Checking if it is a First Person Character overlapping
	AKillingFloorLikeCharacter* Character = Cast<AKillingFloorLikeCharacter>(OtherActor);

	if (Character == nullptr || Character->HasAuthority() == false)
	{
		return;
	}

	OnPickUp.Broadcast(Character);
	//OnComponentBeginOverlap.RemoveDynamic(this, &UTP_PickUpComponent::OnSphereBeginOverlap);

	/*ABaseWeapon* BaseWeapon = Cast<ABaseWeapon>(GetOwner());
	if (Character == nullptr || BaseWeapon == nullptr)
	{
		return;
	}
	

	if (Character->HasActorBegunPlay() == false || Character->GetWeapon(BaseWeapon->GetWeaponType()) != nullptr)
	{
		return;
	}

	if (Character != nullptr)
	{
		// Notify that the actor is being picked up
		//OnPickUp.Broadcast(Character);
		BaseWeapon->AttachWeaponToCharacter(Character);

		// Unregister from the Overlap Event so it is no longer triggered
		OnComponentBeginOverlap.RemoveAll(this);
	}*/
}

/*void UTP_PickUpComponent::AfterDrop(AKillingFloorLikeCharacter* DroppedCharacter)
{
	if (DroppedCharacter->HasAuthority())
	{
		OnComponentBeginOverlap.AddDynamic(this, &UTP_PickUpComponent::OnSphereBeginOverlap);
	}
}*/
