// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
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
