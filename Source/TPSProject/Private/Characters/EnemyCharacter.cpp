// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "TPSProject/TPSProject.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Perception/PawnSensingComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
 	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionObjectType(ECC_SkeletalMesh);
	GetMesh()->SetGenerateOverlapEvents(true);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;

	PawnSensing = CreateDefaultSubobject<UPawnSensingComponent>(TEXT("PawnSensing"));
	PawnSensing->SightRadius = 1000.f;
	PawnSensing->SetPeripheralVisionAngle(30.f);

}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AEnemyCharacter::ReceiveDamage);
	}

	EnemyController = Cast<AAIController>(GetController());
	MoveToTarget(PatrolTarget);

	if (PawnSensing)
	{
		PawnSensing->OnSeePawn.AddDynamic(this, &AEnemyCharacter::PawnDetected);
	}
}

void AEnemyCharacter::ReceiveDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatorController, AActor* DamageCauser)
{
	if (CurHealth <= 0.f)
	{
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Enemy Hit"));
	CurHealth = FMath::Clamp(CurHealth - Damage, 0.f, MaxHealth);

	AActor* Attacker = Cast<AActor>(InstigatorController->GetPawn());
	GetCharacterMovement()->MaxWalkSpeed = 300.f;
	EnemyState = EEnemyState::EES_Chasing;
	MoveToTarget(Attacker);


	// 살아있으면 Hit Reaction 애니메이션 몽타주 재생
	if (CurHealth > 0.f)
	{
		PlayHitReactMontage();
	}
	// 죽었으면 사망 처리
	else
	{
		Death();
	}
}

// 사망 처리
void AEnemyCharacter::Death()
{
	// Death 애니메이션 몽타주 재생
	PlayDeathMontage();

	// 콜리젼 비활성화
	DisableCollision();

	// 수명 설정
	SetLifeSpan(6.f);
}

// Target이 Radius 내에 있는지에 대한 여부 리턴
bool AEnemyCharacter::InTargetRange(AActor* Target, double Radius)
{
	//UE_LOG(LogTemp, Display, TEXT("InTargetRange Check"));
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();

	return DistanceToTarget <= Radius;
}

// Target으로 이동 요청
void AEnemyCharacter::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(15.f);
	FNavPathSharedPtr NavPath;
	EnemyController->MoveTo(MoveRequest);
}

// 순찰 타겟 선택 및 리턴
AActor* AEnemyCharacter::SelectPatrolTarget()
{
	// 현재 타겟을 목표 타겟으로 중복 지정하지 않기 위해 ValidTargets 배열 생성
	TArray<AActor*> ValidTargets;
	for (AActor* Target : PatrolTargets)
	{
		if (Target != PatrolTarget)
		{
			ValidTargets.AddUnique(Target);
		}
	}

	const int32 NumberOfPatrolTargets = ValidTargets.Num();

	if (NumberOfPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumberOfPatrolTargets - 1);
		return ValidTargets[TargetSelection];
	}

	return nullptr;
}

// 폰 감지시 이벤트
void AEnemyCharacter::PawnDetected(APawn* DetectedPawn)
{
	const bool bShouldChaseTarget =
		EnemyState != EEnemyState::EES_Dead &&
		EnemyState != EEnemyState::EES_Chasing &&
		EnemyState < EEnemyState::EES_Attacking &&
		DetectedPawn->ActorHasTag(FName("PlayerCharacter"));

	if (bShouldChaseTarget)
	{
		CombatTarget = DetectedPawn;
		ClearPatrolTimer();
		SetMoveToTarget(
			EEnemyState::EES_Chasing,
			300.f,
			CombatTarget
		);
	}
}

/*
* 애니메이션 몽타주
*/

// Hit React 애니메이션 몽타주 재생
void AEnemyCharacter::PlayHitReactMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}
}

// Death 애니메이션 몽타주 재생
void AEnemyCharacter::PlayDeathMontage()
{
	DeathPose = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	EnemyState = EEnemyState::EES_Dead;
}

// Enemy 공격 함수
void AEnemyCharacter::Attack()
{
	PlayAttackMontage();
}

// Attack 애니메이션 몽타주 재생
int32 AEnemyCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}


// Montage와 해당 Montage의 SectionNames에서 랜덤으로 선택된 것을 재생
int32 AEnemyCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

// Montage의 SectionName을 재생
void AEnemyCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

void AEnemyCharacter::DisableCollision()
{
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemyCharacter::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

void AEnemyCharacter::LoseInterest()
{
	CombatTarget = nullptr;
}

// EnemyState를 EES, MaxWalkSpeed를 MoveSpeed, 이동 목표를 Target으로 전환
void AEnemyCharacter::SetMoveToTarget(EEnemyState EES, float MoveSpeed, AActor* Target)
{
	EnemyState = EES;
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
	MoveToTarget(Target);
}

// PatrolTimer 클리어
void AEnemyCharacter::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

// AttackTimer 시작
void AEnemyCharacter::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemyCharacter::Attack, AttackTime);
}

// AttackTimer 클리어
void AEnemyCharacter::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyState == EEnemyState::EES_Dead) return;

	if (EnemyState > EEnemyState::EES_Patrolling)
	{
		CheckCombatTarget();
	}
	else
	{
		ResetPatrolTarget();
	}
}

// 정찰 목표 재설정 함수
void AEnemyCharacter::ResetPatrolTarget()
{
	// InTargetRange를 체크하고 True면 정찰 목표 재설정
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = SelectPatrolTarget();
		const float WaitTime = FMath::RandRange(WaitBeforeNextPatrolMin, WaitBeforeNextPatrolMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemyCharacter::PatrolTimerFinished, WaitTime);
	}
}


void AEnemyCharacter::CheckCombatTarget()
{
	// 타겟이 추격 범위 밖 > 타겟 놓침
	// CombatTarget이 CombatRadius 내부에 있지 않을 때 > 추적 중지 후 정찰 재개
	if (!InTargetRange(CombatTarget, CombatRadius))
	{
		ClearAttackTimer();
		LoseInterest();
		if (EnemyState == EEnemyState::EES_Engaging)
		{
			SetMoveToTarget(
				EEnemyState::EES_Patrolling,
				30.f,
				PatrolTarget
			);
		}
	}
	// 타겟이 공격 범위 밖, 추격 범위 내부 > 타겟 추격
	// CombatTarget이 AttackRadius 내부에 있지 않고 EES_Chasing 상태가 아닐 때 > 추척 재개
	else if (!InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Chasing)
	{
		ClearAttackTimer();
		SetMoveToTarget(
			EEnemyState::EES_Chasing,
			300.f,
			CombatTarget
		);
	}
	// 타겟이 공격 범위 내부 > 타겟 공격
	// CombatTarget이 AttackRadius 내부에 있고 EES_Attacking 상태가 아닐 때 > 공격 
	else if (InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Attacking)
	{
		StartAttackTimer();
	}
}

void AEnemyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

//void AEnemyCharacter::GetHit(const FVector& ImpactPoint)
//{
//
//}
