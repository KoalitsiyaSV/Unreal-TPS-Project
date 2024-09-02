// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/PlayerCharacterAnimInstance.h"
#include "Characters/PlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TPSProject/Public/Weapons/Weapon.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TPSProject/Public/Types/CombatState.h"

// Initialize Animtation In cpp
void UPlayerCharacterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	PlayerCharacter = Cast<APlayerCharacter>(TryGetPawnOwner());

	//For IK
	Character = Cast<ACharacter>(TryGetPawnOwner());

	if (PlayerCharacter)
	{
		PlayerCharacterMovementComponent = PlayerCharacter->GetCharacterMovement();
	}
}

// Update Animtation In cpp
void UPlayerCharacterAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (PlayerCharacterMovementComponent == nullptr)
	{
		return;
	}

	GroundSpeed = UKismetMathLibrary::VSizeXY(PlayerCharacterMovementComponent->Velocity);

	IsFalling = PlayerCharacterMovementComponent->IsFalling();
	bWeaponEquipped = PlayerCharacter->IsWeaponEquipped();
	EquippedWeapon = PlayerCharacter->GetEquippedWeapon();
	bIsCrouched = PlayerCharacter->bIsCrouched;
	IsSprinting = PlayerCharacter->IsSprinting();
	bAiming = PlayerCharacter->IsAiming();
	TurningInPlace = PlayerCharacter->GetTurningInPlace();
	bRotateRootBone = PlayerCharacter->ShouldRotateRootBone();

	//YawOffset ���ϱ�
	FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PlayerCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	//Lean �� ���ϱ�
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = PlayerCharacter->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 2.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);

	AO_Yaw = PlayerCharacter->GetAO_Yaw();
	AO_Pitch = PlayerCharacter->GetAO_Pitch();

	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && PlayerCharacter->GetMesh())
	{
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), ERelativeTransformSpace::RTS_World);
		FVector OutPosition;
		FRotator OutRotation;
		PlayerCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"), LeftHandTransform.GetLocation(), FRotator::ZeroRotator, OutPosition, OutRotation);
		LeftHandTransform.SetLocation(OutPosition);
		LeftHandTransform.SetRotation(FQuat(OutRotation));

		if (PlayerCharacter->IsLocallyControlled())
		{
			bLocallyControlled = true;
			FTransform RightHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("hand_r"), ERelativeTransformSpace::RTS_World);
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(RightHandTransform.GetLocation(), RightHandTransform.GetLocation() + (RightHandTransform.GetLocation() - PlayerCharacter->GetHitTarget()));
			RightHandRotation = FMath::RInterpTo(RightHandRotation, LookAtRotation, DeltaTime, 30.f);
		}
		
		// �������� �ѱ� ���� ��ġ ������
		//FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));

		//// �ѱ� ������ ������ ����
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		//// �ѱ� ������ HitTarget������ ����
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), PlayerCharacter->GetHitTarget(), FColor::Blue);

	}

	bUseFABRIK = PlayerCharacter->GetCombatSate() == ECombatState::ECS_Unoccupied;
	bUseAimOffsets = PlayerCharacter->GetCombatSate() == ECombatState::ECS_Unoccupied;
	bUseRightHandTransform = PlayerCharacter->GetCombatSate() == ECombatState::ECS_Unoccupied;

	FootIK(DeltaTime);
}

// make FootIK Instance
void UPlayerCharacterAnimInstance::FootIK(float DeltaTime)
{
	// ĳ���Ͱ� �����ϰ� �߶� ������ ���� ��
	if (Character && !IsFalling)
	{
		// �� ���� x,y ��ġ �� Ư�� ���̿��� ������ ���� Ʈ���̽��� ������ ���(�浹 ����, �������� �Ÿ�)
		TTuple<bool, float> FootR = CapsuleDistance("ik_foot_r", Character);
		TTuple<bool, float> FootL = CapsuleDistance("ik_foot_l", Character);

		// ��� �� ���̶� ���� �浹�� �ִٸ�
		if (FootR.Get<0>() || FootL.Get<0>())
		{
			// ���� ���� ����� ���� ����
			const float Select_Float = UKismetMathLibrary::SelectFloat(FootL.Get<1>(), FootR.Get<1>(), FootL.Get<1>() >= FootR.Get<1>());
			// ���� �̵��� ����
			Displacement = FMath::FInterpTo(Displacement, (Select_Float - 98.f) * -1.f, DeltaTime, IKInterpSpeed);

			// �� ���� x, y ��ġ�� ��� ���̿��� ������ ���� Ʈ���̽� ����
			TTuple<bool, float, FVector> FootTraceR = FootLineTrace("ik_foot_r", Character);
			TTuple<bool, float, FVector> FootTraceL = FootLineTrace("ik_foot_l", Character);

			const float DistanceR = FootTraceR.Get<1>();
			const FVector FootRVector(FootTraceR.Get<2>());
			const FRotator MakeRRotation(
				UKismetMathLibrary::DegAtan2(FootRVector.X, FootRVector.Z) * -1.f,
				0.f,
				UKismetMathLibrary::DegAtan2(FootRVector.Y, FootRVector.Z)
			);

			// �� ������ ����
			RRotation = FMath::RInterpTo(RRotation, MakeRRotation, DeltaTime, IKInterpSpeed);
			RIK = FMath::FInterpTo(RIK, (DistanceR - 110.f) / -45.f, DeltaTime, IKInterpSpeed);

			const float DistanceL = FootTraceL.Get<1>();
			const FVector FootLVector(FootTraceL.Get<2>());
			const FRotator MakeLRotation(
				UKismetMathLibrary::DegAtan2(FootLVector.X, FootLVector.Z) * -1.f,
				0.f,
				UKismetMathLibrary::DegAtan2(FootLVector.Y, FootLVector.Z)
			);

			// �� ������ ����
			LRotation = FMath::RInterpTo(LRotation, MakeLRotation, DeltaTime, IKInterpSpeed);
			LIK = FMath::FInterpTo(LIK, (DistanceL - 110.f) / -45.f, DeltaTime, IKInterpSpeed);
		}
	}
	// ĳ���Ͱ� �������� �ʰų� �߶� ���� ��
	else
	{
		LRotation = FMath::RInterpTo(LRotation, FRotator::ZeroRotator, DeltaTime, IKInterpSpeed);
		LIK = FMath::FInterpTo(LIK, 0.f, DeltaTime, IKInterpSpeed);

		RRotation = FMath::RInterpTo(RRotation, FRotator::ZeroRotator, DeltaTime, IKInterpSpeed);
		RIK = FMath::FInterpTo(RIK, 0.f, DeltaTime, IKInterpSpeed);
	}
}

// BreakVector�� �������� �÷��̾��� �߰� ��� �ִ� ������ �浹 ���ο� �Ÿ��� ����
TTuple<bool, float> UPlayerCharacterAnimInstance::CapsuleDistance(FName SocketName, ACharacter* Char)
{
	const FVector WorldLocation{ Char->GetMesh()->GetComponentLocation() };
	const FVector BreakVector{ WorldLocation + FVector(0.f, 0.f, 100.f) };

	const FVector BoneLocation{ Char->GetMesh()->GetSocketLocation(SocketName) };

	// �÷��̾��� ��ġ���� 98.f ���� xy���� foot bone�� ���� ���� Ʈ���̽� ������
	const FVector Start{ BoneLocation.X, BoneLocation.Y, BreakVector.Z };
	// ���������� 151.f��ŭ ���� ������ �浹 �߻� => �ٸ��� ������ �� �ִ� ����
	const FVector End{ Start - FVector(0.f, 0.f, 151.f) };

	FHitResult HitResult;

	// ���� Ʈ���̽� ����
	UKismetSystemLibrary::LineTraceSingle(
		this,
		Start,
		End,
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		IgnoreActors,
		EDrawDebugTrace::None,
		HitResult,
		false
	);

	// ����Ʈ���̽� ����� �浹���� Ȯ��
	const bool Result(HitResult.bBlockingHit);

	// �浹 �߻� ���ο� �������� �Ÿ��� ����
	return MakeTuple(Result, HitResult.Distance);
}

// ��ݿ��� ������ �Ÿ� ���ϱ�
TTuple<bool, float, FVector> UPlayerCharacterAnimInstance::FootLineTrace(FName SocketName, ACharacter* Char)
{
	const FVector BoneLocation{ Char->GetMesh()->GetSocketLocation(SocketName) };
	const FVector RootLocation(Char->GetMesh()->GetBoneLocation("root"));

	// ������ ���ϱ�
	const FVector Start{ BoneLocation.X, BoneLocation.Y, RootLocation.Z };

	FHitResult HitResult;

	// root ���� z�� + 105.f���� ���� -105.f���� ���� Ʈ���̽� ����
	UKismetSystemLibrary::LineTraceSingle(
		this,
		Start + FVector(0.f, 0.f, 105.f),
		Start + FVector(0.f, 0.f, -105.f),
		ETraceTypeQuery::TraceTypeQuery1,
		false,
		IgnoreActors,
		EDrawDebugTrace::None,
		HitResult,
		false,
		FLinearColor::Blue
	);

	const bool Result(HitResult.bBlockingHit);

	// �浹 ���ΰ� true�� �Ÿ��� �浹 ������ ����ȭ�� ���͸� ����
	if (HitResult.bBlockingHit)
	{
		return MakeTuple(Result, HitResult.Distance, HitResult.Normal);
	}
	else
	{
		return MakeTuple(Result, 999.f, FVector::ZeroVector);
	}
}

void UPlayerCharacterAnimInstance::SetPlayerSpeed(float Speed)
{
	PlayerCharacter->SetSpeed(Speed);
}
