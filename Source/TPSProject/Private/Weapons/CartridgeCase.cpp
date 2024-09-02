// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/CartridgeCase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

// ź�� Ŭ���� ������
ACartridgeCase::ACartridgeCase()
{
 	PrimaryActorTick.bCanEverTick = true;

	CaseMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Case Mesh"));
	SetRootComponent(CaseMesh);
	CaseMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	CaseMesh->SetSimulatePhysics(true);
	CaseMesh->SetEnableGravity(true);
	CaseMesh->SetNotifyRigidBodyCollision(true);

	ShellEjectionImpulse = 10.f;
	isHit = false;
}

void ACartridgeCase::BeginPlay()
{
	Super::BeginPlay();
	
	CaseMesh->OnComponentHit.AddDynamic(this, &ACartridgeCase::OnHit);
	CaseMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);

}

// SetTimer�� &&Method::Destroy�� �ȵǼ� �Լ� ����
// �����ؼ� �˾ƺ� ����
void ACartridgeCase::DestroyActor()
{
	Destroy();
}

// ź�ǿ� �浹 �̺�Ʈ �߻� ��
void ACartridgeCase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// �Ҹ��� ù �浹 �ÿ��� ������
	if (ShellSound && !isHit)
	{
		isHit = true;
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}

	// 4�� �ڿ� ź�� �����
	FTimerHandle myTimer;
	GetWorld()->GetTimerManager().SetTimer(myTimer, this, &ACartridgeCase::DestroyActor ,4.f, false);
}

