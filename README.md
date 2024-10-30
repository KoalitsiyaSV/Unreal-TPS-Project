# Unreal-TPS-Project
> 언리얼 기반 협동 TPS 게임 개발 프로젝트

</br>

## 개요
> [영상(임시)](https://drive.google.com/file/d/1zO39WCTPe3NnZ4uzbB1ix0tJd-9QrBTV/view)
</br>
</br>

## 사용 툴
> Unreal Engine 5.2.1  </br>
> Visual Studio 2022  </br>
> Blender3D(간단한 에셋 편집 용도)  </br>
> Aseprite(UI 제작 용도)  </br>

</br>

## 주요 구현 기능
<details>
<summary><b>펼치기</b></summary>
<div markdown="1">

### IK 및 경사에 따른 발 각도 수정
> [Line 102 - Line 240](https://github.com/KoalitsiyaSV/Unreal-TPS-Project/blob/main/Source/TPSProject/Private/Characters/PlayerCharacterAnimInstance.cpp)

</br>

### 하이드 메시
> [Line 448 - line 474](https://github.com/KoalitsiyaSV/Unreal-TPS-Project/blob/main/Source/TPSProject/Private/Characters/PlayerCharacter.cpp)

</details>

</br>
</br>

## 핵심 트러블 슈팅

#### 적이 한 번의 공격에 여러 번 피해를 입히는 문제
> BeginOverlap 이벤트가 여러번 발생  </br>
> Set 자료구조를 활용하여 한 공격에서 피해를 입은 대상인지 체크  </br>
> 이후 공격이 종료되면 Set을 비움으로써 1회 공격에 1회 타격이 되도록 함  </br>
>
> [Line 212 - Line 240, Line 264 - Line 271](https://github.com/KoalitsiyaSV/Unreal-TPS-Project/blob/main/Source/TPSProject/Private/Characters/EnemyCharacter.cpp)

<details><summary>코드</summary>

```cpp
void AEnemyCharacter::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const bool bShouldAttack =
		OtherActor &&
		OtherActor != this &&
		OtherActor->ActorHasTag(FName("PlayerCharacter")) &&
		!OverlappedPlayers.Contains(OtherActor);

	if (bShouldAttack)
	{
		APlayerCharacter* OverlappedCharacter = Cast<APlayerCharacter>(OtherActor);
		if (OverlappedCharacter)
		{
			AController* OverlappedCharacterController = OverlappedCharacter->GetController();
			if (OverlappedCharacterController)
			{
				UGameplayStatics::ApplyDamage(
					OtherActor, 
					AttackDamage, 
					OverlappedCharacterController, 
					this, 
					UDamageType::StaticClass()
				);

				OverlappedPlayers.Add(OtherActor);
			}
		}
	}
}

// Enemy 공격 종료 시
void AEnemyCharacter::AttackEnd()
{
	EnemyState = EEnemyState::EES_NoState;
	AttackPose = -1;
	OverlappedPlayers.Empty();
	CheckCombatTarget();
}
```
</details>

</br>
</br>

## 그 외 트러블 슈팅

<details><summary>VS에서 빌드 시 MSB3073 에러</summary>
1. UFunction() 등 체크 및 수정  </br>
2. 작업관리자를 열어 백그라운드에 남아있는 언리얼 엔진 라이브 코딩 프로세스 종료
</details>

<details><summary>패키징 시 블루스크린/비디오 메모리 부족/패키징 실패</summary>
 연구실에서 사용하는 컴퓨터의 문제, 집에서 패키징하니 문제 없이 한 번에 성공
</details>
