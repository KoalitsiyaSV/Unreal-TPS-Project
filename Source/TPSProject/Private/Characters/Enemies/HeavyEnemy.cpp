// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemies/HeavyEnemy.h"

void AHeavyEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//UE_LOG(LogTemp, Display, TEXT("%d"), static_cast<int32>(EnemyState));
}

void AHeavyEnemy::JumpAttackStart()
{
	ChangeMoveSpeed(300.f);
}

void AHeavyEnemy::JumpAttackEnd()
{
	ChangeMoveSpeed(0.f);
}