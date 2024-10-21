// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/EnemyCharacter.h"
#include "HeavyEnemy.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API AHeavyEnemy : public AEnemyCharacter
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;

protected:
	/*UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	EEnemyState EnemyState = EEnemyState::EES_Patrolling;*/

	//virtual void Attack() override;

	UFUNCTION(BlueprintCallable)
	void JumpAttackStart();

	UFUNCTION(BlueprintCallable)
	void JumpAttackEnd();

private:

public:

};
