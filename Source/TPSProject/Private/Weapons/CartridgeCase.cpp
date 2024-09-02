// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapons/CartridgeCase.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "TimerManager.h"

// 탄피 클래스 생성자
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

// SetTimer에 &&Method::Destroy가 안되서 함수 생성
// 관련해서 알아볼 예정
void ACartridgeCase::DestroyActor()
{
	Destroy();
}

// 탄피에 충돌 이벤트 발생 시
void ACartridgeCase::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	// 소리는 첫 충돌 시에만 낼거임
	if (ShellSound && !isHit)
	{
		isHit = true;
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}

	// 4초 뒤에 탄피 사라짐
	FTimerHandle myTimer;
	GetWorld()->GetTimerManager().SetTimer(myTimer, this, &ACartridgeCase::DestroyActor ,4.f, false);
}

