// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickups/SupplyPickup.h"
#include "TPSProject/Public/Characters/PlayerCharacter.h"
#include "TPSProject/Public/Components/CombatComponent.h"
#include "Components/WidgetComponent.h"

void ASupplyPickup::Destroyed()
{
	Super::Destroyed();

	APlayerCharacter* Character = Cast<APlayerCharacter>(PlayerCharacter);
	if (Character)
	{
		UCombatComponent* Combat = PlayerCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupSupply(WeaponType);
		}
	}
}

void ASupplyPickup::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnSphereOverlap(OverlappedComponent, 
		OtherActor, 
		OtherComp, 
		OtherBodyIndex, 
		bFromSweep, 
		SweepResult
	);
	
	/*APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
	if (PlayerCharacter)
	{
		UCombatComponent* Combat = PlayerCharacter->GetCombat();
		if (Combat)
		{
			Combat->PickupSupply(WeaponType);
		}
	}*/

	//Destroy();
}