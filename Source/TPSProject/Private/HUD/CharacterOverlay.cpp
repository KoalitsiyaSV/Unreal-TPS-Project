// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/CharacterOverlay.h"
#include "TPSProject/Public/Weapons/WeaponTypes.h"
#include "Components/Image.h"
#include "Styling/SlateBrush.h"

void UCharacterOverlay::UpdateHUDWeaponImage(EWeaponType WeaponType)
{
	FSlateBrush Brush;

	switch (WeaponType)
	{
	case EWeaponType::EWT_AssaultRifle:
		Brush.SetResourceObject(ARImage);
		break;
	case EWeaponType::EWT_Shotgun:
		Brush.SetResourceObject(SGImage);
		break;
	case EWeaponType::EWT_SniperRifle:
		Brush.SetResourceObject(SRImage);
		break;
	case EWeaponType::EWT_GrenadeLauncher:
		Brush.SetResourceObject(GLImage);
		break;
	case EWeaponType::EWT_RocketLauncher:
		Brush.SetResourceObject(RLImage);
		break;
	default:
		Brush.SetResourceObject(ARImage);
		break;
	}

	Brush.ImageSize = FVector2D(64.0f, 64.0f);
	WeaponImage->SetBrush(Brush);
}
