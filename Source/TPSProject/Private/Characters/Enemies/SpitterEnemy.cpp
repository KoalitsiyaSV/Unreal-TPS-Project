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

    // ��ƼŬ�� ���Ͽ� �����Ͽ� ����
    UGameplayStatics::SpawnEmitterAttached(
        HitParticles,                      // ������ ��ƼŬ �ý���
        GetMesh(),                         // ������ �޽� ������Ʈ
        HeadSocket,                        // ������ ���� �̸�
        FVector::ZeroVector,               // ��ġ ������ (������ ��������)
        FRotator::ZeroRotator,             // ȸ�� ������ (������ ��������)
        EAttachLocation::SnapToTarget,     // ���� ��ġ�� ���� ����
        true                               // ��ƼŬ�� ������ų�� ����
    );
}

void ASpitterEnemy::SpawnBreathAtLocation()
{
    if (HitParticles)
    {
        FName HeadSocket = TEXT("HeadSocket");

        // HeadSocket�� ��ġ�� ȸ���� ��������
        FVector SocketLocation = GetMesh()->GetSocketLocation(HeadSocket);
        FRotator SocketRotation = GetMesh()->GetSocketRotation(HeadSocket);

        // ĳ������ ���� �������� ��ƼŬ ȸ����Ű��
        FVector ForwardDirection = GetActorForwardVector();
        FVector SpawnLocation = SocketLocation;

        // ��ƼŬ ����
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), HitParticles, SpawnLocation, SocketRotation);

        // Ʈ���̽� ��� ���� ����
        FVector EndLocation = SocketLocation + (ForwardDirection * AttackRange); // 1000 ������ŭ ����
        FHitResult HitResult;

        // Ʈ���̽� �õ� (���� Ʈ���̽� �Ǵ� ���Ǿ� Ʈ���̽��� ��ü ����)
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
                // ������ ������ ����
                APlayerCharacter* OverlappedCharacter = Cast<APlayerCharacter>(OtherActor);
                if (OverlappedCharacter)
                {
                    AController* OverlappedCharacterController = OverlappedCharacter->GetController();
                    if(OverlappedCharacterController)
                    {
                        // ������ ó�� (��: 10�� �������� ��)
                        UGameplayStatics::ApplyDamage(
                            OtherActor,
                            AttackDamage,
                            OverlappedCharacterController,
                            this,
                            UDamageType::StaticClass()
                        );

                        // �ߺ� ó�� ������ ���� OverlappedPlayers�� �߰�
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
