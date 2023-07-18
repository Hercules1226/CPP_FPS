// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerFire.h"
#include "FPSPlayer.h"
#include <Components/ArrowComponent.h>
#include "Bullet.h"
#include <Kismet/GameplayStatics.h>
#include <Camera/CameraComponent.h>
#include "Enemy.h"
#include "EnemyFSM.h"

// Sets default values for this component's properties
UPlayerFire::UPlayerFire()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	bWantsInitializeComponent = true;
}


// Called when the game starts
void UPlayerFire::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


void UPlayerFire::InitializeComponent()
{
	Super::InitializeComponent();

	me = Cast<AFPSPlayer>(GetOwner());
	// player 에 있는 OnInputDelegate 에 처리 함수를 등록하고 싶다.
	me->OnInputDelegate.AddUObject(this, &UPlayerFire::SetupPlayerInputComponent);

	// 총구 컴포넌트 가져오기
	firePosition = Cast<UArrowComponent>(me->GetDefaultSubobjectByName(TEXT("FirePosition")));
}

// Called every frame
void UPlayerFire::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPlayerFire::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	PlayerInputComponent->BindAction(TEXT("Fire"), IE_Pressed, this, &UPlayerFire::Fire);
}

// 사용자가 발사버튼을 누르면 총알을 발사하고 싶다.
void UPlayerFire::Fire()
{
	// 카메라 흔들리도록 처리
	GetWorld()->GetFirstPlayerController()->PlayerCameraManager->StartCameraShake(cameraShake);

	/*GetWorld()->SpawnActor<ABullet>(bulletFactory,
	firePosition->GetComponentLocation(),
	firePosition->GetComponentRotation());*/

	// Line Trace 를 이용하여 충돌처리
	FHitResult hitInfo;

	// 카메라의 위치에서 -> 카메라가 바라보는 방향으로
	auto fpsCam = Cast<UCameraComponent>(me->GetDefaultSubobjectByName(TEXT("FPSCamera")));

	FVector start = fpsCam->GetComponentLocation();
	FVector end = fpsCam->GetComponentLocation() + fpsCam->GetForwardVector() * fireDistance;

	// 특정 액터를 충돌 검출에서 제외시키고 싶을 때
	FCollisionQueryParams param;
	param.AddIgnoredActor(me);
	bool bHit = GetWorld()->LineTraceSingleByChannel(hitInfo, start, end, ECC_Pawn, param);

	// Line 이 부딪혔을 때 부딪힌 지점에 파티클효과 재생
	if (bHit)
	{
		FTransform trans;
		trans.SetLocation(hitInfo.ImpactPoint);
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), bulletEffect, trans);

		// 만약 부딪힌 녀석이 Cube 라면 날려버리고 싶다.
		if (hitInfo.GetActor()->GetName().Contains(TEXT("Cube")))
		{
			auto comp = hitInfo.GetComponent();
			comp->AddForceAtLocation(-hitInfo.ImpactNormal * bulletPower * comp->GetMass(), hitInfo.ImpactPoint);
		}

		// 만약 부딪힌 녀석이 Enemy 라면
		auto enemy = Cast<AEnemy>(hitInfo.GetActor());
		if(enemy)
		{
			// -> 상태를 Damage 로 전환
			//enemy->enemyFSM->m_state = EEnemyState::Damage;
			enemy->enemyFSM->OnDamageProcess(fpsCam->GetForwardVector());
		}
	}
}

