// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/TPSProjectPlayerController.h"
#include "TPSProject/Public/HUD/PlayerHUD.h"
#include "TPSProject/Public/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "TPSProject/Public/Characters/PlayerCharacter.h"
#include "TPSProject/Public/Components/CombatComponent.h"
#include "TPSProject/Public/Weapons/WeaponTypes.h"
#include "Components/Image.h"

ATPSProjectPlayerController::ATPSProjectPlayerController()
	: DefaultColor(0.9f, 0.9f, 0.9f, 0.9f),
	  AlertColor(0.59f, 0.05f, 0.07f, 0.9f)
{
	bGunAndMagHUDIsBlink = false;
}

void ATPSProjectPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetHUD());
}

void ATPSProjectPlayerController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	PollInit();
	//CharacterOverlay->BlinkWidget();
}


void ATPSProjectPlayerController::PollInit()
{
	if (CharacterOverlay == nullptr)
	{
		if (PlayerHUD && PlayerHUD->CharacterOverlay)
		{
			CharacterOverlay = PlayerHUD->CharacterOverlay;
			if (CharacterOverlay)
			{
				APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(GetPawn());
				if (PlayerCharacter && PlayerCharacter->GetCombat())
				{
					SetHUDGrenades(PlayerCharacter->GetCombat()->GetGrenades(), PlayerCharacter->GetCombat()->GetMaxGrenades());
					//SetHUDMaxGrenades(PlayerCharacter->GetCombat()->GetMaxGrenades());
				}
			}
		}
	}
}

// HUD의 체력바 설정
void ATPSProjectPlayerController::SetHUDHealth(float CurHealth, float MaxHealth)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
	
	// HUD가 유효한지 확인
	bool bHUDValid = PlayerHUD && 
		PlayerHUD->CharacterOverlay && 
		PlayerHUD->CharacterOverlay->HealthBar;

	if (bHUDValid)
	{
		const float HealthPercent = CurHealth / MaxHealth;
		PlayerHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		
		
	}
}

// HUD 탄약 표시량 설정
void ATPSProjectPlayerController::SetHUDAmmo(float Ammo, float MagCapacity)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->Mag;

	if (bHUDValid)
	{
		UProgressBar* MagBar = PlayerHUD->CharacterOverlay->Mag;

		const float MagPercent = Ammo / MagCapacity;
		MagBar->SetPercent(MagPercent);

		/*UE_LOG(LogTemp, Warning, TEXT("Ammo : %d"), Ammo);
		UE_LOG(LogTemp, Warning, TEXT("MagCapacity : %d"), MagCapacity);
		UE_LOG(LogTemp, Warning, TEXT("Calc : %f"), static_cast<float>(Ammo) / static_cast<float>(MagCapacity));
		UE_LOG(LogTemp, Warning, TEXT("MagPercent : %f"), MagPercent);*/

		// 현재 잔탄 수에 따른 색상 변경
		if (MagPercent < 0.3f)
		{
			MagBar->SetFillColorAndOpacity(AlertColor);
			PlayerHUD->CharacterOverlay->MagBackGround->SetColorAndOpacity(AlertColor);
			PlayerHUD->CharacterOverlay->WeaponImage->SetColorAndOpacity(AlertColor);
			if (!bGunAndMagHUDIsBlink)
			{
				bGunAndMagHUDIsBlink = true;
				PlayerHUD->CharacterOverlay->BlinkWidget(bGunAndMagHUDIsBlink);
			}
		}
		else
		{
			MagBar->SetFillColorAndOpacity(DefaultColor);
			PlayerHUD->CharacterOverlay->MagBackGround->SetColorAndOpacity(DefaultColor);
			PlayerHUD->CharacterOverlay->WeaponImage->SetColorAndOpacity(DefaultColor);
			if (bGunAndMagHUDIsBlink)
			{
				bGunAndMagHUDIsBlink = false;
				PlayerHUD->CharacterOverlay->BlinkWidget(bGunAndMagHUDIsBlink);
			}
		}
		
		UE_LOG(LogTemp, Warning, TEXT("%s"), bGunAndMagHUDIsBlink ? TEXT("true") : TEXT("false"));

		//UpdateProgressBarAppearance(MagBar, RedColor, RedColor);
	}
}

// HUD 플레이어 탄창 소지량 설정
void ATPSProjectPlayerController::SetHUDMags(int32 Mag, int32 MaxMag)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->Mags &&
		PlayerHUD->CharacterOverlay->MagsX;

	if (bHUDValid)
	{
		UTextBlock* MagsTextBlock = PlayerHUD->CharacterOverlay->Mags;

		FString MagsText = FString::Printf(TEXT("%d/%d"), Mag, MaxMag);
		MagsTextBlock->SetText(FText::FromString(MagsText));

		// 현재 소지량에 따른 색상 변경
		if (Mag < (int32)(MaxMag / 2))
		{
			MagsTextBlock->SetColorAndOpacity(AlertColor);
			PlayerHUD->CharacterOverlay->MagsX->SetColorAndOpacity(AlertColor);
		}
		else
		{
			MagsTextBlock->SetColorAndOpacity(DefaultColor);
			PlayerHUD->CharacterOverlay->MagsX->SetColorAndOpacity(DefaultColor);
		}
	}
}

// HUD 수류탄 소지량 표기
void ATPSProjectPlayerController::SetHUDGrenades(int32 Grenades, int32 MaxGrenades)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->Grenades;

	if (bHUDValid)
	{
		UTextBlock* GrenadesTextBlock = PlayerHUD->CharacterOverlay->Grenades;

		FString GrenadesText = FString::Printf(TEXT("%d/%d"), Grenades, MaxGrenades);
		GrenadesTextBlock->SetText(FText::FromString(GrenadesText));

		// 현재 소지량에 따른 색상 변경
		if (Grenades <= 1)
		{
			GrenadesTextBlock->SetColorAndOpacity(AlertColor);
			PlayerHUD->CharacterOverlay->GrenadesX->SetColorAndOpacity(AlertColor);
			PlayerHUD->CharacterOverlay->GrenadeImage->SetColorAndOpacity(AlertColor);
		}
		else
		{
			GrenadesTextBlock->SetColorAndOpacity(DefaultColor);
			PlayerHUD->CharacterOverlay->GrenadesX->SetColorAndOpacity(DefaultColor);
			PlayerHUD->CharacterOverlay->GrenadeImage->SetColorAndOpacity(DefaultColor);
		}
	}
}

// HUD 스팀팩 소지량 표기
void ATPSProjectPlayerController::SetHUDStimpacks(int32 Stimpacks, int32 MaxStimpacks)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->Stimpacks &&
		PlayerHUD->CharacterOverlay->StimpacksX;

	if (bHUDValid)
	{
		UTextBlock* StimpacksTextBlock = PlayerHUD->CharacterOverlay->Stimpacks;

		FString StimpacksText = FString::Printf(TEXT("%d/%d"), Stimpacks, MaxStimpacks);
		StimpacksTextBlock->SetText(FText::FromString(StimpacksText));

		// 현재 소지량에 따른 색상 변경
		if (Stimpacks <= 1)
		{
			StimpacksTextBlock->SetColorAndOpacity(AlertColor);
			PlayerHUD->CharacterOverlay->Stimpacks->SetColorAndOpacity(AlertColor);
			PlayerHUD->CharacterOverlay->StimpackImage->SetColorAndOpacity(AlertColor);
		}
		else
		{
			StimpacksTextBlock->SetColorAndOpacity(DefaultColor);
			PlayerHUD->CharacterOverlay->Stimpacks->SetColorAndOpacity(DefaultColor);
			PlayerHUD->CharacterOverlay->StimpackImage->SetColorAndOpacity(DefaultColor);
		}
	}
}

void ATPSProjectPlayerController::SetHUDWeaponImage(EWeaponType WeaponType)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->WeaponImage;

	if (bHUDValid)
	{
		PlayerHUD->CharacterOverlay->UpdateHUDWeaponImage(WeaponType);
	}
}
