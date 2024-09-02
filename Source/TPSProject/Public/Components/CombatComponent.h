// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TPSProject/Public/HUD/PlayerHUD.h"
#include "TPSProject/Public/Weapons/WeaponTypes.h"
#include "TPSProject/public/Types/CombatState.h"
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

	void DropEquippedWeapon();
	void AttachActorToRightHand(AActor* ActorToAttach);
	void AttachActorToLeftHand(AActor* ActorToAttach);
	void UpdateCarriedAmmo();
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

	// 조준점 관련 인자들
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

	// 무기 발사 관련 인자
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

	TMap<EWeaponType, int32> MagMap;

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

	void InitializeCarriedMag();

	UPROPERTY(ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState;

	UFUNCTION()
	void OnRep_CombatState();
};
