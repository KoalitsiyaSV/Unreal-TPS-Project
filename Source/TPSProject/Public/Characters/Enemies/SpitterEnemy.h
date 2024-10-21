// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Characters/EnemyCharacter.h"
#include "Components/CapsuleComponent.h"
#include "SpitterEnemy.generated.h"

class UParticleSystem;

UCLASS()
class TPSPROJECT_API ASpitterEnemy : public AEnemyCharacter
{
	GENERATED_BODY()
	
public:
	ASpitterEnemy();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere)
	UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere)
	float BreathTime = 0.1f;

	UPROPERTY(EditAnywhere)
	float AttackRange = 1000.f;

	virtual void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

private:
	/*
	* Range Attack
	*/
	void SpawnBreathParticle();
	void SpawnBreathAtLocation();

	void CollisionEnabled();
	void CollisionDisabled();

	void ResetOvelappedPlayers();

	UFUNCTION(BlueprintCallable)
	void BreathAttackStart();

	UFUNCTION(BlueprintCallable)
	void BreathAttackEnd();

	UPROPERTY(VisibleAnywhere)
	UCapsuleComponent* BreathArea;

	FTimerHandle BreathTimer;
	FTimerHandle BraethHitTimer;

public:


};
