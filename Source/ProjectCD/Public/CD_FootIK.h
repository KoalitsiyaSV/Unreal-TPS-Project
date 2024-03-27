// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CD_FootIK.generated.h"

typedef struct IK_TraceInfo {
	float fOffset;
	FVector pImpactLocation;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTCD_API UCD_FootIK : public UActorComponent
{
	GENERATED_BODY()

public:
	UCD_FootIK();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;
};
