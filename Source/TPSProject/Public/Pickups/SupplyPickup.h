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
	// ¹«±â Å¸ÀÔ
	EWeaponType WeaponType;

	/*
	º¸±ÞÇ° È¹µæÀ¸·Î ¾ò´Â ÅºÃ¢, ¼ö·ùÅº µîÀÇ ¼ö
	*/

	// ÅºÃ¢, Åº¾à
	UPROPERTY(EditAnywhere)
	int32 MagAmount = 4;

	// ¼ö·ùÅº
	UPROPERTY(EditAnywhere)
	int32 GrenadeAmount = 2;

	// ½ºÆÀÆÑ
	UPROPERTY(EditAnywhere)
	int32 StimpackAmount = 2;

};
