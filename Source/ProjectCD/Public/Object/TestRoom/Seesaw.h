// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Seesaw.generated.h"

UCLASS()
class PROJECTCD_API ASeesaw : public AActor
{
	GENERATED_BODY()
	
public:
	virtual void Tick(float DeltaTime) override;

	ASeesaw();

protected:
	virtual void BeginPlay() override;

private:
	void RotatePlatform(float DeltaTime);

	UPROPERTY(EditAnywhere, Category = "Rotation")
	FRotator RotationVelocity;
};
