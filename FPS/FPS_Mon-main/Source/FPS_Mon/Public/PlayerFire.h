// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerFire.generated.h"

// 사용자가 발사버튼을 누르면 총알을 발사하고 싶다.
// 필요속성 : 총알공장
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPS_MON_API UPlayerFire : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerFire();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void InitializeComponent() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);

	void Fire();

	UPROPERTY(EditDefaultsOnly, Category = CameraShake)
	TSubclassOf<class UCameraShakeBase> cameraShake;

public:
	// 필요속성 : 총알공장
	UPROPERTY(EditDefaultsOnly, Category=BulletFactory)
	TSubclassOf<class ABullet> bulletFactory;

	// 필요속성:소유액터
	UPROPERTY()
	class AFPSPlayer* me;

	UPROPERTY()
	class UArrowComponent* firePosition;

	UPROPERTY(EditAnywhere, Category=FireDistance)
	float fireDistance = 1000;

	UPROPERTY(EditDefaultsOnly, Category=BulletEffect)
	class UParticleSystem* bulletEffect;

	// 총을 쐈을 때 부딪힌 물체에 가할 힘
	UPROPERTY(EditDefaultsOnly, Category = BulletPower)
	float bulletPower = 100;
};
