// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapons/ProjectileWeapon.h"
#include "Shotgun.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API AShotgun : public AProjectileWeapon
{
	GENERATED_BODY()

public:
	virtual void Fire(const FVector& HitTarget) override;

private:
	int32 NumberOfPellits = 12;

};
