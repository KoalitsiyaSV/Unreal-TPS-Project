// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/CombatComponent.h"
#include "TPSProject/Public/Weapons/Weapon.h"
#include "TPSProject/Public/Characters/PlayerCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TPSProject/Public/PlayerController/TPSProjectPlayerController.h"
//#include "TPSProject/Public/HUD/PlayerHUD.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character->GetCameraComponent())
	{
		DefaultFOV = Character->GetCameraComponent()->FieldOfView;
		CurrentFOV = DefaultFOV;
	}
	if (Character->HasAuthority())
	{
		InitializeMag();
		
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (Character && Character->IsLocallyControlled())
	{
		FHitResult HitResult;
		TraceUnderCrosshairs(HitResult);
		HitTarget = HitResult.ImpactPoint;

		SetHUDCrosshairs(DeltaTime);
		InterpFOV(DeltaTime);
	}
	
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, bSprinting);
	DOREPLIFETIME(UCombatComponent, bSprintToAim);
	DOREPLIFETIME(UCombatComponent, bAimToSprint);
	DOREPLIFETIME_CONDITION(UCombatComponent, MagAmount, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, Grenades);
}


// ���� ���� ��ȯ
void UCombatComponent::SetSprinting(bool bIsSprinting)
{
	bSprinting = bIsSprinting;

	if (bAiming)
	{
		return;
	}

	if (bSprinting)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = 1000.f;
	}
	else
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = 450.f;
	}

	ServerSetSprinting(bIsSprinting);
}

// ���� ���� ��ȯ(Ŭ���̾�Ʈ)
void UCombatComponent::ServerSetSprinting_Implementation(bool bIsSprinting)
{
	bSprinting = bIsSprinting;

	if (bAiming)
	{
		return;
	}

	if (bSprinting)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = 1000.f;
	}
	else
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = 450.f;
	}
}

// ���� ����
void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	if (CombatState != ECombatState::ECS_Unoccupied) return;
	DropEquippedWeapon();

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachActorToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(Character);
	EquippedWeapon->SetHUDAmmo();
	UpdateCarriedAmmo();
	PlayEquipWeaponSound();
	Character->GetCharacterMovement()->MaxWalkSpeed = 450.f;
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
		AttachActorToRightHand(EquippedWeapon);
		//Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		//Character->bUseControllerRotationYaw = true;
		PlayEquipWeaponSound();
	}
}

// ������ ���Ⱑ ������ ���� ������
void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

// ������ ���Ͽ� ���� ����
void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
}

// �޼� ���Ͽ� ���� ����
void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("LeftHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
}

// ���⺰ �ִ� ź��� ����ǰ ȹ�� �� ź�� ȹ�淮�� �ʱ�ȭ�ϴ� �Լ�
void UCombatComponent::InitializeMag()
{
	// �� ������ �ִ� ź��(Mag/Ammo) ���� MaxAmmoMap�� �ʱ�ȭ
	MaxAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, MaxARMag);
	MaxAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, MaxRLAmmo);
	MaxAmmoMap.Emplace(EWeaponType::EWT_Shotgun, MaxSGAmmo);
	MaxAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, MaxSRMag);
	MaxAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, MaxGLAmmo);

	// �� ������ ����ǰ ȹ�� �� ź��(Mag/Ammo) ȹ�淮�� PickupSupplyAmmoMap�� �ʱ�ȭ
	PickupSupplyAmmoMap.Emplace(EWeaponType::EWT_AssaultRifle, PickupSupplyARMag);
	PickupSupplyAmmoMap.Emplace(EWeaponType::EWT_RocketLauncher, PickupSupplyRLAmmo);
	PickupSupplyAmmoMap.Emplace(EWeaponType::EWT_Shotgun, PickupSupplySGAmmo);
	PickupSupplyAmmoMap.Emplace(EWeaponType::EWT_SniperRifle, PickupSupplySRMag);
	PickupSupplyAmmoMap.Emplace(EWeaponType::EWT_GrenadeLauncher, PickupSupplyGLAmmo);
}

// ������ ���⿡ ���� �ִ� ���� ���� źâ ���� �� ���� ���� źâ ���� ����
void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	if (MaxAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		MaxMagAmount = MaxAmmoMap[EquippedWeapon->GetWeaponType()];
		MagAmount = MaxMagAmount;
	}

	UpdateAmmoHUD();
}

// ź HUD ������Ʈ
void UCombatComponent::UpdateAmmoHUD()
{
	Controller = Controller == nullptr ? Cast<ATPSProjectPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		//Controller->SetHUDMaxMags(MaxMagAmount);
		Controller->SetHUDMags(MagAmount, MaxMagAmount);
	}
}

// ����ǰ�� ȹ�� �� WeaponType�� �´� ź���� ȹ��
void UCombatComponent::PickupSupply(EWeaponType WeaponType)
{
	if (PickupSupplyAmmoMap.Contains(WeaponType))
	{
		MagAmount = FMath::Clamp(MagAmount + PickupSupplyAmmoMap[WeaponType], 0, MaxAmmoMap[WeaponType]);
	}

	UpdateAmmoHUD();

	/*if (MagAmount == 0 && EquippedWeapon && EquippedWeapon->IsEmpty() && EquippedWeapon->GetWeaponType() == WeaponType)
	{
		Reload();
	}*/
}

// ���� ���� ���� ���
void UCombatComponent::PlayEquipWeaponSound()
{
	if (Character && EquippedWeapon && EquippedWeapon->PickupSound)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			EquippedWeapon->PickupSound,
			Character->GetActorLocation()
		);
	}
}

// ���� ���
void UCombatComponent::Reload()
{
	// źâ�� 1�� �̻�, ź�� �������� �ʾ��� ��, ���� Ư�� �ൿ ���� �ƴ� �� ���� ����
	if (MagAmount > 0 && 
		EquippedWeapon->GetAmmo() != EquippedWeapon->GetMagCapacity() && 
		CombatState == ECombatState::ECS_Unoccupied)
	{
		ServerReload();
	}
}

void UCombatComponent::ServerReload_Implementation()
{
	if (Character == nullptr) return;

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

// ���� ����
void UCombatComponent::FinishReloading()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (Character->HasAuthority())
	{
		MagAmount--;
		Controller->SetHUDMags(MagAmount, MaxMagAmount);

		EquippedWeapon->ReloadAmmo();

		CombatState = ECombatState::ECS_Unoccupied;
	}
	if (bFire)
	{
		Fire();
	}
}

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bFire)
		{
			Fire();
		}
		break;
	case ECombatState::ECS_ThrowingGrenade:
		if (Character && !Character->IsLocallyControlled())
		{
			Character->PlayThrowGrenadeMontage();
			AttachActorToLeftHand(EquippedWeapon);
		}
		break;
	}
}

void UCombatComponent::HandleReload()
{
	Character->PlayReloadMontage();
}

// ��� ��ư�� ������ ��
void UCombatComponent::FireWeapon(bool bPressed)
{
	bFire = bPressed;

	if (bFire)
	{
		Fire();
	}
}

// ��� ���� ����
bool UCombatComponent::CanFire()
{
	if (EquippedWeapon == nullptr) return false;
	return !EquippedWeapon->IsEmpty() && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::OnRep_MagAmount()
{
	Controller = Controller == nullptr ? Cast<ATPSProjectPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDMags(MagAmount, MaxMagAmount);
	}
}

// ��� ���
void UCombatComponent::Fire()
{
	if(CanFire())
	{
		bCanFire = false;
		ServerFireWeapon(HitTarget);

		// ��� �� ������ Ȯ��
		CrosshairFireFactor += EquippedWeapon->GetCrosshairFireFactor();

		StartFireTimer();
	}
}

// �߻� Ÿ�̸� ����
void UCombatComponent::StartFireTimer()
{
	if (EquippedWeapon == nullptr || Character == nullptr) return;
	Character->GetWorldTimerManager().SetTimer(
		FireTimer,
		this,
		&UCombatComponent::FireTimerFinished,
		EquippedWeapon->FireDelay
	);
}

// �߻� Ÿ�̸� ����
void UCombatComponent::FireTimerFinished()
{
	if (EquippedWeapon == nullptr) return;
	bCanFire = true;
	if (bFire && EquippedWeapon->bAutomatic)
	{
		Fire();
	}
	ReloadEmptyWeapon();
}

// �� ���� ����
void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon->IsEmpty())
	{
		Reload();
	}
}

void UCombatComponent::ServerFireWeapon_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	MulticastFire(TraceHitTarget);
}

// ��Ƽĳ��Ʈ�� ȣ��Ǹ� ������ ��� Ŭ���̾�Ʈ���� ����
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;

	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

// ������ ����
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	// ����Ʈ ������ ���ϱ�
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// ȭ�� �߾�(= ũ�ν� ��� ��ġ) ���ϱ�
	FVector2D CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if (Character)
		{
			float DistanceToCharacter = (Character->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}

		FVector End = Start + CrosshairWorldDirection * TRACELENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
		);

		HUDPackage.CrosshairColor = FLinearColor::White;

		// �浹�� ���Ͱ� �ְ� �ش� ���Ϳ� InteractWithCrosshairInterface�� �����Ǿ� �ִٸ�
		/*if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}*/

		// �浹 ����� Ž������ ���� �� �Ѿ��� ������ �߽����� �߻�
		// Ž������ �ʾ��� �� �������� ���ư����� �ڵ� �߰�
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		//else
		//{
		//	HitTarget = TraceHitResult.ImpactPoint;
		//	// ȭ�� �߾�
		//	DrawDebugSphere(
		//		GetWorld(),
		//		Start,
		//		6.f,
		//		12,
		//		FColor::Blue
		//	);
		//
		//	// �ٶ󺸰� �ִ� ������ �浹 �߻��ϸ� DebugSphere�� �׸�
		//	DrawDebugSphere(
		//		GetWorld(),
		//		TraceHitResult.ImpactPoint,
		//		6.f,
		//		12,
		//		FColor::Red
		//	);
		//
		//}
	}
}

// ũ�ν� ��� ����
void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;

	Controller = Controller == nullptr ? Cast<ATPSProjectPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		HUD = HUD == nullptr ? Cast<APlayerHUD>(Controller->GetHUD()) : HUD;
		if (HUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairsCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairsLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairsRight = EquippedWeapon->CrosshairRight;
				HUDPackage.CrosshairsTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairsBottom = EquippedWeapon->CrosshairBottom;
			}
			else
			{
				HUDPackage.CrosshairsCenter = nullptr;
				HUDPackage.CrosshairsLeft = nullptr;
				HUDPackage.CrosshairsRight = nullptr;
				HUDPackage.CrosshairsTop = nullptr;
				HUDPackage.CrosshairsBottom = nullptr;
			}

			// �÷��̾� �ӵ��� ���� ũ�ν� ��� ������ ����
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			// �ӵ��� ���� ������ Ȯ�� ����
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			// ���߿� ���� ���� ������ Ȯ�� ����
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			// ���� ���� ������ Ȯ�� ����
			// ���� �ÿ��� Ȯ�굵�� �پ����
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			// ������ �ÿ��� Ȯ�굵�� �þ����
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			// ��� ���� ������ Ȯ�� ����
			CrosshairFireFactor = FMath::FInterpTo(CrosshairFireFactor, 0.f, DeltaTime, 40.f);

			// ������ Ȯ�� ��ġ
			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairFireFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}

}

// ���� ���� ��ȯ
void UCombatComponent::SetAiming(bool bIsAiming)
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;

	bAiming = bIsAiming;
	//Character->GetCharacterMovement()->bOrientRotationToMovement = !bIsAiming;
	/*if (Character->GetVelocity().Size() > 0.f || Character->GetMovementComponent()->IsFalling())
	{
		Character->bUseControllerRotationYaw = bIsAiming;
	}*/

	if (bAiming)
	{
		if (Character->bIsCrouched)
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = 250.f;
		}
		else
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = 350.f;
		}
	}
	else
	{
		if (Character->bIsCrouched)
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = 300.f;
		}
		else
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = 450.f;
		}
	}

	ServerSetAiming(bIsAiming);

	if (Character->IsLocallyControlled() && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
	{
		Character->ShowSRScopeWidget(bIsAiming);
		UE_LOG(LogTemp, Warning, TEXT("bIsAiming: %s"), bIsAiming ? TEXT("True") : TEXT("false"));

	}
}

// ���� ���� ��ȯ(Ŭ���̾�Ʈ)
void UCombatComponent::ServerSetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	//Character->GetCharacterMovement()->bOrientRotationToMovement = !bIsAiming;
	/*if (Character->GetVelocity().Size() > 0.f || Character->GetMovementComponent()->IsFalling())
	{
		Character->bUseControllerRotationYaw = bIsAiming;
	}*/

	if (bAiming)
	{
		if (Character->bIsCrouched)
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = 250.f;
		}
		else
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = 350.f;
		}
	}
	else
	{
		if (Character->bIsCrouched)
		{
			Character->GetCharacterMovement()->MaxWalkSpeedCrouched = 300.f;
		}
		else
		{
			Character->GetCharacterMovement()->MaxWalkSpeed = 450.f;
		}
	}
}

// FOV ����
void UCombatComponent::InterpFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr) return;

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime, EquippedWeapon->GetZoomedInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultFOV, DeltaTime, ZoomInterpSpeed);
	}

	if (Character && Character->GetCameraComponent())
	{
		Character->GetCameraComponent()->SetFieldOfView(CurrentFOV);
	}
}

/* 
����ź ��ô ���
*/

// ����ź ��ô �ִϸ��̼� ����
void UCombatComponent::ThrowGrenade()
{
	if (Grenades == 0) return;
	if (CombatState != ECombatState::ECS_Unoccupied || EquippedWeapon == nullptr) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		//EquippedWeapon->GetWeaponMesh()->SetHiddenInGame(true);
		// ����ϴ� �ִϸ��̼� �� ���⸦ ���ִ� �� �����ε�
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}

	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}

	if (Character && Character->HasAuthority())
	{
		Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
		UpdateHUDGrenade();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	if (Grenades == 0) return;
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		//EquippedWeapon->GetWeaponMesh()->SetHiddenInGame(false);
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}

	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateHUDGrenade();
}

// ������ ����ź�� Visibility ����
void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

// ����ź ��ô ����
void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

// ����ź ��ô �� ��ô�Ǵ� ����ź ����
void UCombatComponent::ThrowGrenadeSpawn()
{
	ShowAttachedGrenade(false);
	if (Character && Character->IsLocallyControlled())
	{
		ServerThrowGrenadeSpawn(HitTarget);
	}
}

void UCombatComponent::ServerThrowGrenadeSpawn_Implementation(const FVector_NetQuantize& Target)
{
	if (Character && GrenadeClass && Character->GetAttachedGrenade())
	{
		// ����ź ���� ��ġ = ���� ĳ���� �𵨸��� �����Ǿ� �ִ� ����ź�� ��ġ
		const FVector StartingLoaction = Character->GetAttachedGrenade()->GetComponentLocation();

		// Ÿ�� ��ġ
		FVector ToTarget = Target - StartingLoaction;

		// FActorSpawnParameters ����ü ����
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = Character;
		SpawnParams.Instigator = Character;

		UE_LOG(LogTemp, Display, TEXT("Throw"));

		// ����
		UWorld* World = GetWorld();
		if (World)
		{
			World->SpawnActor<AProjectile>(
				GrenadeClass,
				StartingLoaction,
				ToTarget.Rotation(),
				SpawnParams
			);
		}
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateHUDGrenade();

}

// ����ź ���� HUD ������Ʈ
void UCombatComponent::UpdateHUDGrenade()
{
	Controller = Controller == nullptr ? Cast<ATPSProjectPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDGrenades(Grenades, MaxGrenades);
	}
}