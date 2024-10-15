// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UENUM(BlueprintType)
enum class EDeathPose : uint8
{
	EDP_Death1 UMETA(DisplayName = "Death1"),
	EDP_Death2 UMETA(DisplayName = "Death2"),
	EDP_Death3 UMETA(DisplayName = "Death3"),
	EDP_Death4 UMETA(DisplayName = "Death4"),
	EDP_Death5 UMETA(DisplayName = "Death5"),
	EDP_Death6 UMETA(DisplayName = "Death6"),
	EDP_Death7 UMETA(DisplayName = "Death7"),
	EDP_Death8 UMETA(DisplayName = "Death8"),

	EDP_MAX UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EEnemyState : uint8
{
	EES_Dead UMETA(DisplayName = "Dead"),
	EES_Patrolling UMETA(DisplayName = "Patrolling"),
	EES_Chasing UMETA(DisplayName = "Chasing"),
	EES_Attacking UMETA(DisplayName = "Attacking"),
	EES_Engaging UMETA(DisplayName = "Engaging"),
	EES_Sleeping UMETA(DisplayName = "Sleeping")
};

class UAnimMontage;
class UPawnSensingComponent;

UCLASS()
class TPSPROJECT_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();
	virtual void Tick(float DeltaTime) override;
	void ResetPatrolTarget();
	void CheckCombatTarget();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//virtual void GetHit(const FVector& ImpactPoint) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, class AController* InstigatorController, AActor* DamageCauser);

	void Death();
	bool InTargetRange(AActor* Target, double Radius);
	void MoveToTarget(AActor* Target);
	AActor* SelectPatrolTarget();
	void Attack();


	UFUNCTION()
	void PawnDetected(APawn* DetectedPawn);

	/*
	�� �������ͽ�
	*/

	// �ִ� ü��
	UPROPERTY(EditAnywhere, Category = "Enemy Status")
	float MaxHealth = 100.f;

	// ���� ü��
	UPROPERTY(VisibleAnywhere, Category = "Enemy Status")
	float CurHealth = 100.f;


	/*
	* �ִϸ��̼� ��Ÿ�� ���� �Լ�
	*/
	void PlayHitReactMontage();
	void PlayDeathMontage();
	int32 PlayAttackMontage();
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	void DisableCollision();

	UPROPERTY(BlueprintReadOnly)
	int32 DeathPose = -1;

	UPROPERTY(BlueprintReadOnly)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;


private:
	/*
	������Ʈ
	*/
	UPROPERTY(VisibleAnywhere)
	UPawnSensingComponent* PawnSensing;

	/*
	�ִϸ��̼� ��Ÿ��
	*/
	UPROPERTY(EditDefaultsOnly, Category = Montage)
	UAnimMontage* HitReactMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montage)
	UAnimMontage* DeathMontage;

	UPROPERTY(EditDefaultsOnly, Category = Montage)
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, Category = Montage)
	TArray<FName>  DeathMontageSections;

	UPROPERTY(EditAnywhere, Category = Montage)
	TArray<FName>  AttackMontageSections;


	UPROPERTY()
	AActor* CombatTarget;

	UPROPERTY(EditAnywhere)
	double CombatRadius = 1000.f;

	UPROPERTY(EditAnywhere)
	double AttackRadius = 150.f;

	/*
	 �׺���̼�
	*/
	UPROPERTY()
	class AAIController* EnemyController;

	// ���� ���� ����
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	AActor* PatrolTarget;

	// ���� ���� �迭
	UPROPERTY(EditInstanceOnly, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

	UPROPERTY(EditAnywhere)
	double PatrolRadius = 200.f;

	FTimerHandle PatrolTimer;
	void PatrolTimerFinished();

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitBeforeNextPatrolMin = 5.f;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	float WaitBeforeNextPatrolMax = 10.f;

	/*
	* �� �ൿ
	*/
	void LoseInterest();
	void SetMoveToTarget(EEnemyState EES, float MoveSpeed, AActor* Target);
	void ClearPatrolTimer();

	/*
	* ����
	*/
	void StartAttackTimer();
	void ClearAttackTimer();

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMin = 0.5f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float AttackMax = 1.4f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float PatrollingSpeed = 30.f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ChasingSpeed = 300.f;

public:	

};
