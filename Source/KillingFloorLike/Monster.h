// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Monster.generated.h"


class UPoolableComponent;

UENUM(BlueprintType)
enum class EMonsterType : uint8
{
	None,
	Clot,
	Gorefast,
	Stalker,
	Crawler,
	Husk,
	Scrake,
	Fleshpound,
	Siren,
	Bloat
};

ENUM_RANGE_BY_COUNT(EMonsterType, EMonsterType::Bloat)


UENUM(BlueprintType)
enum class EMonsterAnimationType : uint8
{
	None,
	Idle,
	Walk,
	Run,
	Attack,
	Hit,
	Skill,
	HeadOff,
	Stun
};

USTRUCT(BlueprintType)
struct FMonsterMontages
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<UAnimMontage*> Montages;
};


USTRUCT(BlueprintType)
struct FMonsterData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 몬스터 처치 보상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	int32 id;
	
	// 몬스터 유형
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	EMonsterType type;

	// 몬스터 처치 보상
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	int32 bounty;

	// 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	int32 health;

	// 머리 체력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	int32 head_health;

	// 공격력
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	float damage;

	// 이동 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	float speed;

	// 달리기 속도
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	float run_speed;

	// 공격 범위
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	float range;

	// 스턴 한계치
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	float stun_threshold;

	// 출혈 지속 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Data")
	int32 bleed_out_time;

	FMonsterData()
		: type(EMonsterType::None)
		  , bounty(0)
		  , health(0)
		  , head_health(0)
		  , damage(0.0f)
		  , speed(0.0f)
		  , run_speed(0.0f)
		  , range(0.0f)
		  , stun_threshold(0.0f)
		  , bleed_out_time(0)
	{
	}
};


UCLASS()
class KILLINGFLOORLIKE_API AMonster : public ABaseCharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AMonster();


protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator,
	                         AActor* DamageCauser) override;

private:
	UPROPERTY()
	class UKFLikeGameInstance* GameInstance;
	
	UPROPERTY(EditAnywhere)
	TMap<EMonsterAnimationType, FMonsterMontages> AnimationMap;

	/*UPROPERTY(EditAnywhere)
	TMap<EMonsterSoundType, USoundBase*> SoundMap;*/

	UPROPERTY(EditAnywhere, BlueprintGetter = GetMonsterType)
	EMonsterType MonsterType;
	UPROPERTY(EditAnywhere, Category="Data")
	FMonsterData MonsterData;

	UPROPERTY(EditAnywhere, Category="Stat")
	float CurrentHeadHP;

	UPROPERTY(Replicated)
	FName HeadBoneName;

	FTimerHandle TimerHandle;

	UPROPERTY()
	class AAIController* AIController;
	/*UPROPERTY()
	class UBehaviorTreeComponent* BehaviourTree;*/

	int32 GetId();

	// AMonster.h
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_HideHeadBoneAndExplode(FVector HeadLocation, FName BoneToHide, AActor* DamageCauser);

	UFUNCTION()
	void OnPooledActivate();
	UFUNCTION()
	void OnPooledReset();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UPoolableComponent* PoolableComponent;
	
	UFUNCTION(BlueprintCallable)
	virtual void Dead(AActor* DamageCauser) override;
	virtual void Multi_DeadCallback(AActor* DamageCauser) override;

	virtual void Attack() override;

public:
	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetAnimationMontage(EMonsterAnimationType animType, int32 index);
	UFUNCTION(BlueprintCallable)
	int32 GetAnimationMaxIndex(EMonsterAnimationType animType);


	UFUNCTION(BlueprintCallable)
	UAnimMontage* GetHitAnimMontage(AActor* DamageCauser);


	UFUNCTION(BlueprintGetter)
	EMonsterType GetMonsterType();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetBleedOutTime();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetStunThreshold();

	UFUNCTION(BlueprintImplementableEvent)
	void HeadOff(AActor* DamageCauser);

	void SetMonsterType(EMonsterType NewMonsterType);
	void SetMonsterData(const FMonsterData& NewMonsterData);

	void SetBBIsMoveable(bool IsMoveable);
};
