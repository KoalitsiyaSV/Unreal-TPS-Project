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
		InitializeCarriedMag();
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
}


// 질주 상태 전환
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

// 질주 상태 전환(클라이언트)
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

// 무기 장착
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

// 장착된 무기가 있으면 무기 떨구기
void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Dropped();
	}
}

// 오른손 소켓에 액터 부착
void UCombatComponent::AttachActorToRightHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
}

// 왼손 소켓에 액터 부착
void UCombatComponent::AttachActorToLeftHand(AActor* ActorToAttach)
{
	if (Character == nullptr || Character->GetMesh() == nullptr || ActorToAttach == nullptr) return;
	const USkeletalMeshSocket* HandSocket = Character->GetMesh()->GetSocketByName(FName("LeftHandSocket"));
	if (HandSocket)
	{
		HandSocket->AttachActor(EquippedWeapon, Character->GetMesh());
	}
}

// 최대 소지 가능 탄창 개수 및 현재 소지 탄창 개수 설정
void UCombatComponent::UpdateCarriedAmmo()
{
	if (EquippedWeapon == nullptr) return;
	if (MagMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		MaxMagAmount = MagMap[EquippedWeapon->GetWeaponType()];
		MagAmount = MaxMagAmount;
	}

	Controller = Controller == nullptr ? Cast<ATPSProjectPlayerController>(Character->Controller) : Controller;
	if (Controller)
	{
		Controller->SetHUDMaxMagAmount(MaxMagAmount);
		Controller->SetHUDMagAmount(MagAmount);
	}
}

// 무기 장착 사운드 재생
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

// 장전 기능
void UCombatComponent::Reload()
{
	// 탄창이 1개 이상, 탄이 가득차지 않았을 때, 현재 특정 행동 중이 아닐 때 장전 가능
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

// 장전 종료
void UCombatComponent::FinishReloading()
{
	if (Character == nullptr || EquippedWeapon == nullptr) return;
	if (Character->HasAuthority())
	{
		MagAmount--;
		Controller->SetHUDMagAmount(MagAmount);

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

// 사격 버튼이 눌려질 시
void UCombatComponent::FireWeapon(bool bPressed)
{
	bFire = bPressed;

	if (bFire)
	{
		Fire();
	}
}

// 사격 가능 여부
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
		Controller->SetHUDMagAmount(MagAmount);
	}
}

void UCombatComponent::InitializeCarriedMag()
{
	MagMap.Emplace(EWeaponType::EWT_AssaultRifle, MaxARMag);
	MagMap.Emplace(EWeaponType::EWT_RocketLauncher, MaxRLAmmo);
	MagMap.Emplace(EWeaponType::EWT_Shotgun, MaxSGAmmo);
	MagMap.Emplace(EWeaponType::EWT_SniperRifle, MaxSRMag);
	MagMap.Emplace(EWeaponType::EWT_GrenadeLauncher, MaxGLAmmo);
}

// 사격 기능
void UCombatComponent::Fire()
{
	if(CanFire())
	{
		bCanFire = false;
		ServerFireWeapon(HitTarget);

		// 사격 시 조준점 확산
		CrosshairFireFactor += EquippedWeapon->GetCrosshairFireFactor();

		StartFireTimer();
	}
}

// 발사 타이머 시작
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

// 발사 타이머 종료
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

// 빈 무기 장전
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

// 멀티캐스트가 호출되면 서버와 모든 클라이언트에서 실행
void UCombatComponent::MulticastFire_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;

	if (Character && CombatState == ECombatState::ECS_Unoccupied)
	{
		Character->PlayFireMontage(bAiming);
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

// 조준점 추적
void UCombatComponent::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	// 뷰포트 사이즈 구하기
	FVector2D ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}

	// 화면 중앙(= 크로스 헤어 위치) 구하기
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

		// 충돌한 액터가 있고 해당 액터에 InteractWithCrosshairInterface가 구현되어 있다면
		/*if (TraceHitResult.GetActor() && TraceHitResult.GetActor()->Implements<UInteractWithCrosshairInterface>())
		{
			HUDPackage.CrosshairColor = FLinearColor::Red;
		}
		else
		{
			HUDPackage.CrosshairColor = FLinearColor::White;
		}*/

		// 충돌 결과가 탐지되지 않을 때 총알이 월드의 중심으로 발사
		// 탐지되지 않았을 때 끝점으로 날아가도록 코드 추가
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		//else
		//{
		//	HitTarget = TraceHitResult.ImpactPoint;
		//	// 화면 중앙
		//	DrawDebugSphere(
		//		GetWorld(),
		//		Start,
		//		6.f,
		//		12,
		//		FColor::Blue
		//	);
		//
		//	// 바라보고 있는 지점에 충돌 발생하면 DebugSphere를 그림
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

// 크로스 헤어 설정
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

			// 플레이어 속도에 따른 크로스 헤어 퍼지는 정도
			FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D VelocityMultiplierRange(0.f, 1.f);
			FVector Velocity = Character->GetVelocity();
			Velocity.Z = 0.f;

			// 속도에 따른 조준점 확산 인자
			CrosshairVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

			// 공중에 있을 때의 조준점 확산 인자
			if (Character->GetCharacterMovement()->IsFalling())
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 2.25f, DeltaTime, 2.25f);
			}
			else
			{
				CrosshairInAirFactor = FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);
			}

			// 조준 시의 조준점 확산 인자
			// 조준 시에는 확산도가 줄어야함
			if (bAiming)
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.58f, DeltaTime, 30.f);
			}
			// 비조준 시에는 확산도가 늘어야함
			else
			{
				CrosshairAimFactor = FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);
			}

			// 사격 시의 조준점 확산 인자
			CrosshairFireFactor = FMath::FInterpTo(CrosshairFireFactor, 0.f, DeltaTime, 40.f);

			// 조준점 확산 수치
			HUDPackage.CrosshairSpread = 0.5f + CrosshairVelocityFactor + CrosshairInAirFactor - CrosshairAimFactor + CrosshairFireFactor;

			HUD->SetHUDPackage(HUDPackage);
		}
	}

}

// 조준 상태 전환
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

// 조준 상태 전환(클라이언트)
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

// FOV 설정
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

// 수류탄 투척 기능
void UCombatComponent::ThrowGrenade()
{
	if (CombatState != ECombatState::ECS_Unoccupied) return;

	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		//EquippedWeapon->GetWeaponMesh()->SetHiddenInGame(true);
		// 사용하는 애니메이션 상 무기를 없애는 건 별로인듯
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}

	if (Character && !Character->HasAuthority())
	{
		ServerThrowGrenade();
	}
}

void UCombatComponent::ServerThrowGrenade_Implementation()
{
	CombatState = ECombatState::ECS_ThrowingGrenade;
	if (Character)
	{
		Character->PlayThrowGrenadeMontage();
		//EquippedWeapon->GetWeaponMesh()->SetHiddenInGame(false);
		AttachActorToLeftHand(EquippedWeapon);
		ShowAttachedGrenade(true);
	}

}

void UCombatComponent::ShowAttachedGrenade(bool bShowGrenade)
{
	if (Character && Character->GetAttachedGrenade())
	{
		Character->GetAttachedGrenade()->SetVisibility(bShowGrenade);
	}
}

void UCombatComponent::ThrowGrenadeFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachActorToRightHand(EquippedWeapon);
}

void UCombatComponent::ThrowGrenadeSpawn()
{
	ShowAttachedGrenade(false);
}
