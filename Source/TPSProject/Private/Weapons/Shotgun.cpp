// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/Shotgun.h"
#include "Weapons/ProjectileWeapon.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Weapons/Projectile.h"

// ¼¦°Ç 
// 

void AShotgun::Fire(const FVector& HitTarget)
{
	AWeapon::Fire(HitTarget);

	if (!HasAuthority()) return;

	APawn* InstigatorPawn = Cast<APawn>(GetOwner());

	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(FName("MuzzleFlash"));
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		// From Muzzle flash socket to hit location from TraceUnderCrosshairs
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();

		// Æç¸´ °³¼ö¸¸Å­ ¼öÇà
		for (int32 i = 0; i < NumberOfPellits; i++)
		{
			float RandPitch = FMath::RandRange(-WeaponSpread, WeaponSpread);
			float RandYaw = FMath::RandRange(-WeaponSpread, WeaponSpread);
			FRotator PelletRotation = TargetRotation;
			PelletRotation.Pitch += RandPitch;
			PelletRotation.Yaw += RandYaw;

			//UE_LOG(LogTemp, Warning, TEXT("RandPitch: %f, RandYaw: %f"), RandPitch, RandYaw);

			if (ProjectileClass && InstigatorPawn)
			{
				FActorSpawnParameters SpawnParams;
				SpawnParams.Owner = GetOwner();
				SpawnParams.Instigator = InstigatorPawn;
				UWorld* World = GetWorld();
				if (World)
				{
					World->SpawnActor<AProjectile>(
						ProjectileClass,
						SocketTransform.GetLocation(),
						PelletRotation,
						SpawnParams
					);
				}
			}
		}


	}
}
