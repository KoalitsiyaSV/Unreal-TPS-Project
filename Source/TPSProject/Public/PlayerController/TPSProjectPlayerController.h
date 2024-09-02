// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "TPSProjectPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TPSPROJECT_API ATPSProjectPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	void SetHUDHealth(float CurHealth, float MaxHealth);
	void SetHUDAmmo(int32 Ammo);
	void SetHUDMagAmount(int32 Mag);
	void SetHUDMaxMagAmount(int32 MaxMag);
	void SetWidgetAngleTest();

protected:
	virtual void BeginPlay() override;

private:
	class APlayerHUD* PlayerHUD;
};
