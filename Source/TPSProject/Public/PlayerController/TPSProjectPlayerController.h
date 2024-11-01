// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Components/ProgressBar.h"
#include "TPSProject/Public/Weapons/WeaponTypes.h"
#include "TPSProjectPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API ATPSProjectPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ATPSProjectPlayerController();
	void SetHUDHealth(float CurHealth, float MaxHealth);
	void SetHUDAmmo(float Ammo, float MagCapacity);
	//void UpdateProgressBarAppearance(UProgressBar* ProgressBar, const FLinearColor& FillColor, const FLinearColor& BackgroundColor);
	void SetHUDMags(int32 Mag, int32 MaxMag);
	//void SetHUDMaxMags(int32 MaxMag);
	//void SetWidgetAngleRotationTest();
	void SetHUDGrenades(int32 Grenades, int32 MaxGrenades);
	//void SetHUDMaxGrenades(int32 MaxGrenades);
	void SetHUDStimpacks(int32 Stimpacks, int32 MaxStimpacks);
	void SetHUDWeaponImage(EWeaponType WeaponType);

	virtual void Tick(float DeltaTime) override;

	/*
	* ȿ��
	*/
	UPROPERTY(BlueprintReadWrite)
	bool bGunAndMagHUDIsBlink;

	bool bGrenadeHUDIsBlink;

protected:
	virtual void BeginPlay() override;
	void PollInit();

private:
	class APlayerHUD* PlayerHUD;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	/*
	* ����
	*/
	const FLinearColor DefaultColor;
	const FLinearColor AlertColor;
};