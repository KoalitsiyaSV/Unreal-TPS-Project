// Fill out your copyright notice in the Description page of Project Settings.


#include "CD_FootIK.h"

UCD_FootIK::UCD_FootIK()
{
	PrimaryComponentTick.bCanEverTick = true;
	
}


// Called when the game starts
void UCD_FootIK::BeginPlay()
{
	Super::BeginPlay();
	
}


// Called every frame
void UCD_FootIK::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

