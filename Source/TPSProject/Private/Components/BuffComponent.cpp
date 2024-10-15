// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/BuffComponent.h"
#include "TPSProject/Public/Characters/PlayerCharacter.h"
#include "TimerManager.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}

/*
스팀팩 관련
*/
// HealingTime동안 0.5초마다 HealAmount만큼 체력 회복
void UBuffComponent::Heal(/*float HealAmount, float HealingTime*/)
{
	// 이미 회복중인 경우 리턴
	if (bHealing)
	{
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("Start Healing"));

	bHealing = true;

	// ProcessHealing을 HealingTick마다 반복
	GetWorld()->GetTimerManager().SetTimer(HealTimerHandle, this, &UBuffComponent::ProcessHealing, HealingTick, true);

	// HealingTime초 후 EndHealing 호출
	GetWorld()->GetTimerManager().SetTimer(HealingEndTimerHandle, this, &UBuffComponent::EndHealing, HealingTime, false);
}

void UBuffComponent::ProcessHealing()
{
	if (Character)
	{
		//UE_LOG(LogTemp, Warning, TEXT("Now Healing"));

		float CharacterHealth = Character->GetCurHealth();
		Character->SetCurHealth(CharacterHealth + HealAmount);
	}
}

void UBuffComponent::EndHealing()
{
	GetWorld()->GetTimerManager().ClearTimer(HealTimerHandle);
	GetWorld()->GetTimerManager().ClearTimer(HealingEndTimerHandle);
	bHealing = false;
}

void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//HealRampUp(DeltaTime);
}

