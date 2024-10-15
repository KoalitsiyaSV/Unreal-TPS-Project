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
������ ����
*/
// HealingTime���� 0.5�ʸ��� HealAmount��ŭ ü�� ȸ��
void UBuffComponent::Heal(/*float HealAmount, float HealingTime*/)
{
	// �̹� ȸ������ ��� ����
	if (bHealing)
	{
		return;
	}

	//UE_LOG(LogTemp, Warning, TEXT("Start Healing"));

	bHealing = true;

	// ProcessHealing�� HealingTick���� �ݺ�
	GetWorld()->GetTimerManager().SetTimer(HealTimerHandle, this, &UBuffComponent::ProcessHealing, HealingTick, true);

	// HealingTime�� �� EndHealing ȣ��
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

