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

	//YawOffset 구하기
	FRotator AimRotation = PlayerCharacter->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(PlayerCharacter->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 6.f);
	YawOffset = DeltaRotation.Yaw;

	//Lean 값 구하기
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
		
		// 조준점과 총구 방향 일치 측정용
		//FTransform MuzzleTipTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("MuzzleFlash"), ERelativeTransformSpace::RTS_World);
		//FVector MuzzleX(FRotationMatrix(MuzzleTipTransform.GetRotation().Rotator()).GetUnitAxis(EAxis::X));

		//// 총구 끝에서 나가는 직선
		//DrawDebugLine(GetWorld(), MuzzleTipTransform.GetLocation(), MuzzleTipTransform.GetLocation() + MuzzleX * 1000.f, FColor::Red);
		//// 총구 끝에서 HitTarget까지의 직선
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
	// 캐릭터가 존재하고 추락 중이지 않을 때
	if (Character && !IsFalling)
	{
		// 각 본의 x,y 위치 및 특정 높이에서 땅까지 라인 트레이스를 수행한 결과(충돌 여부, 땅까지의 거리)
		TTuple<bool, float> FootR = CapsuleDistance("ik_foot_r", Character);
		TTuple<bool, float> FootL = CapsuleDistance("ik_foot_l", Character);

		// 어느 한 발이라도 땅과 충돌해 있다면
		if (FootR.Get<0>() || FootL.Get<0>())
		{
			// 땅과 가장 가까운 발을 선택
			const float Select_Float = UKismetMathLibrary::SelectFloat(FootL.Get<1>(), FootR.Get<1>(), FootL.Get<1>() >= FootR.Get<1>());
			// 발의 이동을 보간
			Displacement = FMath::FInterpTo(Displacement, (Select_Float - 98.f) * -1.f, DeltaTime, IKInterpSpeed);

			// 각 본의 x, y 위치의 골반 높이에서 땅까지 라인 트레이스 수행
			TTuple<bool, float, FVector> FootTraceR = FootLineTrace("ik_foot_r", Character);
			TTuple<bool, float, FVector> FootTraceL = FootLineTrace("ik_foot_l", Character);

			const float DistanceR = FootTraceR.Get<1>();
			const FVector FootRVector(FootTraceR.Get<2>());
			const FRotator MakeRRotation(
				UKismetMathLibrary::DegAtan2(FootRVector.X, FootRVector.Z) * -1.f,
				0.f,
				UKismetMathLibrary::DegAtan2(FootRVector.Y, FootRVector.Z)
			);

			// 각 값들을 보간
			RRotation = FMath::RInterpTo(RRotation, MakeRRotation, DeltaTime, IKInterpSpeed);
			RIK = FMath::FInterpTo(RIK, (DistanceR - 110.f) / -45.f, DeltaTime, IKInterpSpeed);

			const float DistanceL = FootTraceL.Get<1>();
			const FVector FootLVector(FootTraceL.Get<2>());
			const FRotator MakeLRotation(
				UKismetMathLibrary::DegAtan2(FootLVector.X, FootLVector.Z) * -1.f,
				0.f,
				UKismetMathLibrary::DegAtan2(FootLVector.Y, FootLVector.Z)
			);

			// 각 값들을 보간
			LRotation = FMath::RInterpTo(LRotation, MakeLRotation, DeltaTime, IKInterpSpeed);
			LIK = FMath::FInterpTo(LIK, (DistanceL - 110.f) / -45.f, DeltaTime, IKInterpSpeed);
		}
	}
	// 캐릭터가 존재하지 않거나 추락 중일 때
	else
	{
		LRotation = FMath::RInterpTo(LRotation, FRotator::ZeroRotator, DeltaTime, IKInterpSpeed);
		LIK = FMath::FInterpTo(LIK, 0.f, DeltaTime, IKInterpSpeed);

		RRotation = FMath::RInterpTo(RRotation, FRotator::ZeroRotator, DeltaTime, IKInterpSpeed);
		RIK = FMath::FInterpTo(RIK, 0.f, DeltaTime, IKInterpSpeed);
	}
}

// BreakVector를 기준으로 플레이어의 발과 딛고 있는 땅과의 충돌 여부와 거리를 구함
TTuple<bool, float> UPlayerCharacterAnimInstance::CapsuleDistance(FName SocketName, ACharacter* Char)
{
	const FVector WorldLocation{ Char->GetMesh()->GetComponentLocation() };
	const FVector BreakVector{ WorldLocation + FVector(0.f, 0.f, 100.f) };

	const FVector BoneLocation{ Char->GetMesh()->GetSocketLocation(SocketName) };

	// 플레이어의 위치보다 98.f 높고 xy값이 foot bone과 같은 곳이 트레이스 시작점
	const FVector Start{ BoneLocation.X, BoneLocation.Y, BreakVector.Z };
	// 시작점보다 151.f만큼 낮은 곳까지 충돌 발생 => 다리가 굽혀질 수 있는 범위
	const FVector End{ Start - FVector(0.f, 0.f, 151.f) };

	FHitResult HitResult;

	// 라인 트레이스 수행
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

	// 라인트레이스 결과가 충돌인지 확인
	const bool Result(HitResult.bBlockingHit);

	// 충돌 발생 여부와 땅까지의 거리를 리턴
	return MakeTuple(Result, HitResult.Distance);
}

// 골반에서 땅까지 거리 구하기
TTuple<bool, float, FVector> UPlayerCharacterAnimInstance::FootLineTrace(FName SocketName, ACharacter* Char)
{
	const FVector BoneLocation{ Char->GetMesh()->GetSocketLocation(SocketName) };
	const FVector RootLocation(Char->GetMesh()->GetBoneLocation("root"));

	// 시작점 구하기
	const FVector Start{ BoneLocation.X, BoneLocation.Y, RootLocation.Z };

	FHitResult HitResult;

	// root 본의 z값 + 105.f에서 부터 -105.f까지 라인 트레이스 수행
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

	// 충돌 여부가 true면 거리와 충돌 지점의 정규화된 벡터를 리턴
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
