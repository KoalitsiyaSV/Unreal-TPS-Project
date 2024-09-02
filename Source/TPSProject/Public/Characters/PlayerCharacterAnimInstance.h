// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "TPSProject/Public/Types/TurningInPlace.h"
#include "PlayerCharacterAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API UPlayerCharacterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	// Initialize Animtation In cpp
	virtual void NativeInitializeAnimation() override;
	// Update Animtation In cpp
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	//FootIK in cpp

	void FootIK(float DeltaTime);
	TTuple<bool, float> CapsuleDistance(FName SocketName, ACharacter* Char);
	TTuple<bool, float, FVector> FootLineTrace(FName SocketName, ACharacter* Char);

	UPROPERTY(BlueprintReadOnly)
	class APlayerCharacter* PlayerCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	class UCharacterMovementComponent* PlayerCharacterMovementComponent;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float GroundSpeed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool IsFalling;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	bool IsSprinting;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	class AWeapon* EquippedWeapon;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bAiming;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float Lean;

	UFUNCTION(BlueprintCallable, Category = Movement, meta = (AllowPrivateAccess = "true"))
	void SetPlayerSpeed(float Speed);

	FRotator CharacterRotationLastFrame;
	FRotator CharacterRotation;
	FRotator DeltaRotation;

	// Aim Offset에 사용
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;

	// 
	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FTransform LeftHandTransform;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	ETurningInPlace TurningInPlace;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	FRotator RightHandRotation;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bLocallyControlled;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bRotateRootBone;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseFABRIK;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseAimOffsets;

	UPROPERTY(BlueprintReadOnly, Category = Movement, meta = (AllowPrivateAccess = "true"))
	bool bUseRightHandTransform;

public:
	//FootIk Instance
	class ACharacter* Character;

	UPROPERTY()
	TArray<AActor*> IgnoreActors;

	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	float Displacement;

	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	float IKInterpSpeed = 15;

	// 밟고 있는 땅의 각도에 따른 발의 회전값
	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	FRotator RRotation;

	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	FRotator LRotation;

	// 무릎이 굽혀지는 정도
	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	float RIK;

	UPROPERTY(BlueprintReadOnly, Category = "IK", meta = (AllowPrivateAccess = "true"))
	float LIK;
};
