// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerController/TPSProjectPlayerController.h"
#include "TPSProject/Public/HUD/PlayerHUD.h"
#include "TPSProject/Public/HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void ATPSProjectPlayerController::BeginPlay()
{
	Super::BeginPlay();

	PlayerHUD = Cast<APlayerHUD>(GetHUD());
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
void ATPSProjectPlayerController::SetHUDAmmo(int32 Ammo)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->AmmoAmount;

	if (bHUDValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("Here"));

		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		PlayerHUD->CharacterOverlay->AmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

// HUD 플레이어 탄창 소지량 설정
void ATPSProjectPlayerController::SetHUDMagAmount(int32 Mag)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->MagAmount;

	if (bHUDValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("Here"));

		FString MagText = FString::Printf(TEXT("%d"), Mag);
		PlayerHUD->CharacterOverlay->MagAmount->SetText(FText::FromString(MagText));
	}
}

// HUD 플레이어 탄창 소지 최대값 설정
void ATPSProjectPlayerController::SetHUDMaxMagAmount(int32 MaxMag)
{
	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;

	bool bHUDValid = PlayerHUD &&
		PlayerHUD->CharacterOverlay &&
		PlayerHUD->CharacterOverlay->MaxMagAmount;

	if (bHUDValid)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Here"));

		FString MaxMagText = FString::Printf(TEXT("%d"), MaxMag);
		PlayerHUD->CharacterOverlay->MaxMagAmount->SetText(FText::FromString(MaxMagText));
	}
}

//void ATPSProjectPlayerController::SetWidgetAngleTest()
//{
//	PlayerHUD = PlayerHUD == nullptr ? Cast<APlayerHUD>(GetHUD()) : PlayerHUD;
//
//	bool bHUDValid = PlayerHUD &&
//		PlayerHUD->CharacterOverlay &&
//		PlayerHUD->CharacterOverlay->HealthBar;
//
//	if (bHUDValid)
//	{
//		// 현재 각도를 가져옵니다.
//		float WidgetAngle = PlayerHUD->CharacterOverlay->HealthBar->RenderTransform.Angle;
//
//		UE_LOG(LogTemp, Warning, TEXT("%f"), WidgetAngle);
//
//		// 각도를 90도 증가시킵니다.
//		WidgetAngle += 90.f;
//
//		// HealthBar의 슬레이트 변환을 업데이트합니다.
//		PlayerHUD->CharacterOverlay->HealthBar->SetRenderTransformAngle(WidgetAngle);
//
//		PlayerHUD->CharacterOverlay->HealthBar->SynchronizeProperties();
//	}
//}