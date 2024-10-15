// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSProject/Public/HUD/PlayerHUD.h"
#include "TPSProject/Public/Weapons/WeaponTypes.h"
#include "TPSProject/public/Types/CombatState.h"
#include "TPSProject/public/Weapons/Projectile.h"
#include "CombatComponent.generated.h"

#define TRACELENGTH 100000.f

class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSPROJECT_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	friend class APlayerCharacter;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void EquipWeapon(AWeapon* WeaponToEquip);
	void Reload();

	UFUNCTION(BlueprintCallable)
	void FinishReloading();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeFinished();

	UFUNCTION(BlueprintCallable)
	void ThrowGrenadeSpawn();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenadeSpawn(const FVector_NetQuantize& Target);

	// º¸±ÞÇ° È¹µæ
	void PickupSupply(EWeaponType WeaponType);

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);
	void SetSprinting(bool bIsSprinting);


	UFUNCTION(Server, Reliable)
	void ServerSetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerSetSprinting(bool bIsSprinting);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void FireWeapon(bool bPressed);

	void Fire();

	UFUNCTION(Server, Reliable)
	void ServerFireWeapon(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrosshairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	void HandleReload();

	void ThrowGrenade();

	UFUNCTION(Server, Reliable)
	void ServerThrowGrenade();

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AProjectile> GrenadeClass;

	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
	void UpdateAmmoHUD();
	void PlayEquipWeaponSound();

	void ReloadEmptyWeapon();
	void ShowAttachedGrenade(bool bShowGrenade);

private:
	class APlayerCharacter* Character;
	class ATPSProjectPlayerController* Controller;
	class APlayerHUD* HUD;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(Replicated)
	bool bSprinting;

	UPROPERTY(Replicated)
	bool bSprintToAim;

	UPROPERTY(Replicated)
	bool bAimToSprint;

	bool bFire;

	// Á¶ÁØÁ¡ °ü·Ã ÀÎÀÚµé
	float CrosshairVelocityFactor;
	float CrosshairInAirFactor;
	float CrosshairAimFactor;
	float CrosshairFireFactor;

	FVector HitTarget;

	FHUDPackage HUDPackage;

	// Field Of View
	float DefaultFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomedFOV = 30.f;

	float CurrentFOV;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float ZoomInterpSpeed = 20.f;

	void InterpFOV(float DeltaTime);

	// ¹«±â ¹ß»ç °ü·Ã ÀÎÀÚ
	FTimerHandle FireTimer;

	bool bCanFire = true;

	void StartFireTimer();
	void FireTimerFinished();
	bool CanFire();

	UPROPERTY(ReplicatedUsing = OnRep_MagAmount)
	int32 MagAmount;

	UPROPERTY()
	int32 MaxMagAmount;

	UFUNCTION()
	void OnRep_MagAmount();

	// °¢ ¹«±âÀÇ ÃÖ´ë Åº ¼ÒÁö·®
	TMap<EWeaponType, int32> MaxAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 MaxARMag = 7;

	UPROPERTY(EditAnywhere)
	int32 MaxRLAmmo = 3;

	UPROPERTY(EditAnywhere)
	int32 MaxSGAmmo = 32;

	UPROPERTY(EditAnywhere)
	int32 MaxSRMag = 4;

	UPROPERTY(EditAnywhere)
	int32 MaxGLAmmo = 7;

	// º¸±ÞÇ° È¹µæ ½Ã Åº È¹µæ·®
	TMap<EWeaponType, int32> PickupSupplyAmmoMap;

	UPROPERTY(EditAnywhere)
	int32 PickupSupplyARMag = 4;

	UPROPERTY(EditAnywhere)
	int32 PickupSupplyRLAmmo = 2;

	UPROPERTY(EditAnywhere)
	int32 PickupSupplySGAmmo = 24;

	UPROPERTY(EditAnywhere)
	int32 PickupSupplySRMag = 2;

	UPROPERTY(EditAnywhere)
	int32 PickupSupplyGLAmmo = 4;

	void InitializeMag();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState;

	UFUNCTION()
	void OnRep_CombatState();

	UPROPERTY(ReplicatedUsing = OnRep_Grenades)
	int32 Grenades = 4;

	UFUNCTION()
	void OnRep_Grenades();

	UPROPERTY(EditAnywhere)
	int32 MaxGrenades = 4;

	void UpdateHUDGrenade();

public:
	FORCEINLINE int32 GetGrenades() const { return Grenades; }
	FORCEINLINE int32 GetMaxGrenades() const { return MaxGrenades; }
};
