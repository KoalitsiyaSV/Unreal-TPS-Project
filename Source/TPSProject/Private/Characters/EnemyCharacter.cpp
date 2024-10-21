// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/EnemyCharacter.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "TPSProject/TPSProject.h"
#include "Kismet/KismetMathLibrary.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AIController.h"
#include "Perception/PawnSensingComponent.h"
#include "TPSProject/Public/Characters/PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"

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

	LHandComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Left Hand Component"));
	LHandComponent->SetupAttachment(GetMesh(), FName("LeftHand"));
	LHandComponent->SetSphereRadius(12.f);
	LHandComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	RHandComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Right Hand Component"));
	RHandComponent->SetupAttachment(GetMesh(), FName("RightHand"));
	RHandComponent->SetSphereRadius(12.f);
	RHandComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		OnTakeAnyDamage.AddDynamic(this, &AEnemyCharacter::ReceiveDamage);
		LHandComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnComponentBeginOverlap);
		RHandComponent->OnComponentBeginOverlap.AddDynamic(this, &AEnemyCharacter::OnComponentBeginOverlap);
	}

	CurHealth = MaxHealth;

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
	/*GetCharacterMovement()->MaxWalkSpeed = 300.f;
	EnemyState = EEnemyState::EES_Chasing;
	MoveToTarget(Attacker);*/

	SetMoveToTarget(
		EEnemyState::EES_Chasing,
		ChasingSpeed,
		Attacker
	);

	// ��������� Hit Reaction �ִϸ��̼� ��Ÿ�� ���
	if (CurHealth > 0.f)
	{
		PlayHitReactMontage();
	}
	// �׾����� ��� ó��
	else
	{
		Death();
	}
}

// ��� ó��
void AEnemyCharacter::Death()
{
	// EnemyState�� Dead�� ����
	EnemyState = EEnemyState::EES_Dead;

	// AttackTimer Ŭ����
	ClearAttackTimer();

	// Death �ִϸ��̼� ��Ÿ�� ���
	PlayDeathMontage();

	// �ݸ��� ��Ȱ��ȭ
	DisableCollision();

	// ���� ����
	SetLifeSpan(DeathLifeSpawn);

	// ��ü ȸ�� ����
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

// Target�� Radius ���� �ִ����� ���� ���� ����
bool AEnemyCharacter::InTargetRange(AActor* Target, double Radius)
{
	//UE_LOG(LogTemp, Display, TEXT("InTargetRange Check"));
	if (Target == nullptr) return false;
	const double DistanceToTarget = (Target->GetActorLocation() - GetActorLocation()).Size();

	return DistanceToTarget <= Radius;
}

// Target���� �̵� ��û
void AEnemyCharacter::MoveToTarget(AActor* Target)
{
	if (EnemyController == nullptr || Target == nullptr) return;

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(15.f);
	FNavPathSharedPtr NavPath;
	EnemyController->MoveTo(MoveRequest);
}

// ���� Ÿ�� ���� �� ����
AActor* AEnemyCharacter::SelectPatrolTarget()
{
	// ���� Ÿ���� ��ǥ Ÿ������ �ߺ� �������� �ʱ� ���� ValidTargets �迭 ����
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

// �� ������ �̺�Ʈ
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
			ChasingSpeed,
			CombatTarget
		);
	}
}

/*
* �ִϸ��̼� ��Ÿ��
*/

// Hit React �ִϸ��̼� ��Ÿ�� ���
void AEnemyCharacter::PlayHitReactMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
	}
}

// Death �ִϸ��̼� ��Ÿ�� ���
void AEnemyCharacter::PlayDeathMontage()
{
	DeathPose = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	EnemyState = EEnemyState::EES_Dead;
}

void AEnemyCharacter::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const bool bShouldAttack =
		OtherActor &&
		OtherActor != this &&
		OtherActor->ActorHasTag(FName("PlayerCharacter")) &&
		!OverlappedPlayers.Contains(OtherActor);

	if (bShouldAttack)
	{
		APlayerCharacter* OverlappedCharacter = Cast<APlayerCharacter>(OtherActor);
		if (OverlappedCharacter)
		{
			AController* OverlappedCharacterController = OverlappedCharacter->GetController();
			if (OverlappedCharacterController)
			{
				UGameplayStatics::ApplyDamage(
					OtherActor, 
					AttackDamage, 
					OverlappedCharacterController, 
					this, 
					UDamageType::StaticClass()
				);

				OverlappedPlayers.Add(OtherActor);
			}
		}
	}
}

// ���� �� �� �ݸ��� Ȱ��ȭ
void AEnemyCharacter::HandCollisionEnabled()
{
	LHandComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RHandComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

// ���� ���� �� �� �ݸ��� ��Ȱ��ȭ
void AEnemyCharacter::HandCollisionDisabled()
{
	LHandComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RHandComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

// Enemy ���� �Լ�
void AEnemyCharacter::Attack()
{
	ChangeMoveSpeed(0.f);
	//EnemyState = EEnemyState::EES_Engaging;
	AttackPose = PlayAttackMontage();
}

// Enemy ���� ���� ��
void AEnemyCharacter::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	AttackPose = -1;
	OverlappedPlayers.Empty();
	CheckCombatTarget();
}

// Attack �ִϸ��̼� ��Ÿ�� ���
int32 AEnemyCharacter::PlayAttackMontage()
{
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}


// Montage�� �ش� Montage�� SectionNames���� �������� ���õ� ���� ���
int32 AEnemyCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (SectionNames.Num() <= 0) return -1;
	const int32 MaxSectionIndex = SectionNames.Num() - 1;
	const int32 Selection = FMath::RandRange(0, MaxSectionIndex);
	PlayMontageSection(Montage, SectionNames[Selection]);
	return Selection;
}

// Montage�� SectionName�� ���
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

// EnemyState�� EES, MaxWalkSpeed�� MoveSpeed, �̵� ��ǥ�� Target���� ��ȯ
void AEnemyCharacter::SetMoveToTarget(EEnemyState EES, float MoveSpeed, AActor* Target)
{
	EnemyState = EES;
	ChangeMoveSpeed(MoveSpeed);
	MoveToTarget(Target);
}

// PatrolTimer Ŭ����
void AEnemyCharacter::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemyCharacter::ChangeMoveSpeed(float MoveSpeed)
{
	GetCharacterMovement()->MaxWalkSpeed = MoveSpeed;
}

// AttackTimer ����
void AEnemyCharacter::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	const float AttackTime = FMath::RandRange(AttackMin, AttackMax);
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemyCharacter::Attack, AttackTime);
}

// AttackTimer Ŭ����
void AEnemyCharacter::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

/*
* EnemyState ���� ����
*/
bool AEnemyCharacter::IsDead()
{
	return EnemyState == EEnemyState::EES_Dead;
}

bool AEnemyCharacter::IsPatrolling()
{
	return EnemyState == EEnemyState::EES_Patrolling;
}

bool AEnemyCharacter::IsChasing()
{
	return EnemyState == EEnemyState::EES_Chasing;
}

bool AEnemyCharacter::IsAttacking()
{
	return EnemyState == EEnemyState::EES_Attacking;
}

bool AEnemyCharacter::IsEngaging()
{
	return EnemyState == EEnemyState::EES_Engaging;
}

void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyState == EEnemyState::EES_Dead) return;

	if (!IsAttacking())
	{
		//UE_LOG(LogTemp, Display, TEXT("check combat target"));
		CheckCombatTarget();
	}
	
	if(IsPatrolling())
	{
		ResetPatrolTarget();
	}

	// ���¿� ���� �ӵ� ���� ������ ������

}

// ���� ��ǥ �缳�� �Լ�
void AEnemyCharacter::ResetPatrolTarget()
{
	// InTargetRange�� üũ�ϰ� True�� ���� ��ǥ �缳��
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = SelectPatrolTarget();
		const float WaitTime = FMath::RandRange(WaitBeforeNextPatrolMin, WaitBeforeNextPatrolMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemyCharacter::PatrolTimerFinished, WaitTime);
	}
}


void AEnemyCharacter::CheckCombatTarget()
{
	// Ÿ���� �߰� ���� �� > Ÿ�� ��ħ
	// CombatTarget�� CombatRadius ���ο� ���� ���� �� > ���� ���� �� ���� �簳
	if (!InTargetRange(CombatTarget, CombatRadius))
	{
		ClearAttackTimer();
		LoseInterest();
		//if (EnemyState == EEnemyState::EES_Engaging)
		//{
			SetMoveToTarget(
				EEnemyState::EES_Patrolling,
				PatrollingSpeed,
				PatrolTarget
			);
		//}
	}
	// Ÿ���� ���� ���� ��, �߰� ���� ���� > Ÿ�� �߰�
	// CombatTarget�� AttackRadius ���ο� ���� �ʰ� EES_Chasing ���°� �ƴ� �� > ��ô �簳
	else if (!InTargetRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Chasing)
	{
		ClearAttackTimer();
		SetMoveToTarget(
			EEnemyState::EES_Chasing,
			ChasingSpeed,
			CombatTarget
		);

	}
	// Ÿ���� ���� ���� ���� > Ÿ�� ����
	// CombatTarget�� AttackRadius ���ο� �ְ� EES_Attacking ���°� �ƴ� �� > ���� 
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
