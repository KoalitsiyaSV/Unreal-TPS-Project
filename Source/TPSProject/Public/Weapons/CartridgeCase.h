// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CartridgeCase.generated.h"

UCLASS()
class TPSPROJECT_API ACartridgeCase : public AActor
{
	GENERATED_BODY()
	
public:	
	ACartridgeCase();

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	virtual void BeginPlay() override;

private:	
	UFUNCTION()
	void DestroyActor();

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CaseMesh;

	UPROPERTY(EditAnywhere)
	float ShellEjectionImpulse;

	UPROPERTY(EditAnywhere)
	class USoundCue* ShellSound;


	bool isHit;
};
