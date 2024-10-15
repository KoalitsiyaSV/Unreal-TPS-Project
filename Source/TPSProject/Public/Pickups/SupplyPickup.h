// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "TPSProject/public/Weapons/WeaponTypes.h"
#include "SupplyPickup.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API ASupplyPickup : public APickup
{
	GENERATED_BODY()

public:
	virtual void Destroyed();

protected:
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	// ���� Ÿ��
	EWeaponType WeaponType;

	/*
	����ǰ ȹ������ ��� źâ, ����ź ���� ��
	*/

	// źâ, ź��
	UPROPERTY(EditAnywhere)
	int32 MagAmount = 4;

	// ����ź
	UPROPERTY(EditAnywhere)
	int32 GrenadeAmount = 2;

	// ������
	UPROPERTY(EditAnywhere)
	int32 StimpackAmount = 2;

};
