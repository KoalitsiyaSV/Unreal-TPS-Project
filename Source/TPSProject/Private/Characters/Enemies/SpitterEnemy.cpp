// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/Enemies/SpitterEnemy.h"
#include "Kismet/GameplayStatics.h"
#include "TPSProject/Public/Characters/PlayerCharacter.h"

ASpitterEnemy::ASpitterEnemy()
{
    BreathArea = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Breath Area"));
    BreathArea->SetupAttachment(GetMesh(), FName("HeadSocket"));
    BreathArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASpitterEnemy::BeginPlay()
{
    Super::BeginPlay();

    if (HasAuthority())
    {
        BreathArea->OnComponentBeginOverlap.AddDynamic(this, &ASpitterEnemy::OnComponentBeginOverlap);
    }
}

void ASpitterEnemy::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    Super::OnComponentBeginOverlap(
        OverlappedComponent,
        OtherActor,
        OtherComp,
        OtherBodyIndex,
        bFromSweep,
        SweepResult
    );

    GetWorldTimerManager().SetTimer(BreathTimer, this, &ASpitterEnemy::ResetOvelappedPlayers, 0.3f, true);
}

void ASpitterEnemy::SpawnBreathParticle()
{
    FName HeadSocket = TEXT("HeadSocket");

    // 파티클을 소켓에 부착하여 생성
    UGameplayStatics::SpawnEmitterAttached(
        HitParticles,                      // 생성할 파티클 시스템
        GetMesh(),                         // 부착할 메쉬 컴포넌트
        HeadSocket,                        // 부착할 소켓 이름
        FVector::ZeroVector,               // 위치 오프셋 (소켓을 기준으로)
        FRotator::ZeroRotator,             // 회전 오프셋 (소켓을 기준으로)
        EAttachLocation::SnapToTarget,     // 소켓 위치에 맞춰 생성
        true                               // 파티클을 부착시킬지 여부
    );
}

void ASpitterEnemy::SpawnBreathAtLocation()
{
    if (HitParticles)
    {
        FName HeadSocket = TEXT("HeadSocket");

        // HeadSocket의 위치와 회전값 가져오기
        FVector SocketLocation = GetMesh()->GetSocketLocation(HeadSocket);
        FRotator SocketRotation = GetMesh()->GetSocketRotation(HeadSocket);

        // 캐릭터의 전방 방향으로 파티클 회전시키기
        FVector ForwardDirection = GetActorForwardVector();
        FVector SpawnLocation = SocketLocation;

        // 파티클 생성
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, SpawnLocation, SocketRotation);

        // 트레이스 기반 공격 판정
        FVector EndLocation = SocketLocation + (ForwardDirection * AttackRange); // 1000 단위만큼 전방
        FHitResult HitResult;

        // 트레이스 시도 (라인 트레이스 또는 스피어 트레이스로 교체 가능)
        GetWorld()->LineTraceSingleByChannel(HitResult, SocketLocation, EndLocation, ECC_Pawn);

        if (HitResult.bBlockingHit)
        {
            AActor* OtherActor = HitResult.GetActor();

            bool bIsPlayer =
                OtherActor &&
                OtherActor->ActorHasTag(FName("PlayerCharacter")) &&
                !OverlappedPlayers.Contains(OtherActor);

            if (bIsPlayer)
            {
                // 적에게 데미지 적용
                APlayerCharacter* OverlappedCharacter = Cast<APlayerCharacter>(OtherActor);
                if (OverlappedCharacter)
                {
                    AController* OverlappedCharacterController = OverlappedCharacter->GetController();
                    if(OverlappedCharacterController)
                    {
                        // 데미지 처리 (예: 10의 데미지를 줌)
                        UGameplayStatics::ApplyDamage(
                            OtherActor,
                            AttackDamage,
                            OverlappedCharacterController,
                            this,
                            UDamageType::StaticClass()
                        );

                        // 중복 처리 방지를 위해 OverlappedPlayers에 추가
                        OverlappedPlayers.Add(OtherActor);
                    }
                }
            }
        }
    }
}

void ASpitterEnemy::CollisionEnabled()
{
    BreathArea->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ASpitterEnemy::CollisionDisabled()
{
    BreathArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASpitterEnemy::ResetOvelappedPlayers()
{
    UE_LOG(LogTemp, Display, TEXT("Overlapped Target Reseted"));
    OverlappedPlayers.Empty();
}

void ASpitterEnemy::BreathAttackStart()
{
    GetWorldTimerManager().SetTimer(BreathTimer, this, &ASpitterEnemy::SpawnBreathAtLocation, BreathTime, true);
    GetWorldTimerManager().SetTimer(BraethHitTimer, this, &ASpitterEnemy::ResetOvelappedPlayers, 1.f, true);
    //CollisionEnabled();
}

void ASpitterEnemy::BreathAttackEnd()
{
    GetWorldTimerManager().ClearTimer(BreathTimer);
    GetWorldTimerManager().ClearTimer(BraethHitTimer);
    //CollisionDisabled();
}
