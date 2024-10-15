// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TPSProject/Public/Weapons/WeaponTypes.h"
#include "CharacterOverlay.generated.h"

class UTextBlock;

/**
 * 
 */
UCLASS()
class TPSPROJECT_API UCharacterOverlay : public UUserWidget
{
	GENERATED_BODY()

public:
	//virtual void Tick(float DeltaTime) override;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* HealthBar;

	/*
	 ÃÑ
	*/

	UPROPERTY(meta = (BindWidget))
	class UImage* WeaponImage;

	UPROPERTY(EditAnywhere)
	UTexture2D* ARImage;

	UPROPERTY(EditAnywhere)
	UTexture2D* SGImage;

	UPROPERTY(EditAnywhere)
	UTexture2D* SRImage;

	UPROPERTY(EditAnywhere)
	UTexture2D* GLImage;

	UPROPERTY(EditAnywhere)
	UTexture2D* RLImage;

	/*
	 Åº ¹× ÅºÃ¢
	*/

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoAmount;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* Mag;

	UPROPERTY(meta = (BindWidget))
	UImage* MagBackGround;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Mags;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MagsX;

	/*UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxMags;*/

	/*
	 ¼ö·ùÅº
	*/

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Grenades;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GrenadesX;

	UPROPERTY(meta = (BindWidget))
	UImage* GrenadeImage;

	/*UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxGrenades;*/

	/*
	 ½ºÆÀÆÑ
	*/

	UPROPERTY(meta = (BindWidget))
	UTextBlock* Stimpacks;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StimpacksX;

	UPROPERTY(meta = (BindWidget))
	UImage* StimpackImage;

	UFUNCTION(BlueprintImplementableEvent)
	void BlinkWidget(bool bisBlinkGunAndMagHUD);

	UFUNCTION()
	void UpdateHUDWeaponImage(EWeaponType WeaponType);

protected:

private:
};