// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TPSPROJECT_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class APlayerCharacter;

	/*
	¹öÇÁ È¿°ú
	*/
	void Heal(/*float HealAmount, float HealingTime*/);

protected:
	virtual void BeginPlay() override;
	//void HealRampUp(float DeltaTime);

private:
	UPROPERTY()
	APlayerCharacter* Character;

	/*
	½ºÆÀÆÑ °ü·Ã
	*/
	FTimerHandle HealTimerHandle;
	FTimerHandle HealingEndTimerHandle;

	bool bHealing = false;
	float HealingTime = 4.f;
	float HealAmount = .2f;
	float HealingTicksRemaining = HealingTime / 0.5f;
	float HealingTick = 0.01f;

	void ProcessHealing();
	void EndHealing();

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE bool GetbHealing() const { return bHealing; }
	FORCEINLINE void SetbHealing(bool bIsHealing) { bHealing = bIsHealing; }
		
};
