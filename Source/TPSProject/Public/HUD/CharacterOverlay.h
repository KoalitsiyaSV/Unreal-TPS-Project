// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
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

	UPROPERTY(meta = (BindWidget))
	UTextBlock* AmmoAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MagAmount;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaxMagAmount;
};
