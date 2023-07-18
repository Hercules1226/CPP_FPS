// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "FPSPlayer.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam( FPlayerInputDelegate , class UInputComponent*);

UCLASS()
class FPS_MON_API AFPSPlayer : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


public:
	UPROPERTY(VisibleAnywhere, Category=FPSCamera)
	class UCameraComponent* fpsCamera;

	UPROPERTY(VisibleAnywhere, Category = BodyMesh)
	class USkeletalMeshComponent* bodyMesh;

	UPROPERTY(VisibleAnywhere, Category = PlayerMove)
	class UPlayerMove* playerMove;

	UPROPERTY(VisibleAnywhere, Category = PlayerFire)
	class UPlayerFire* playerFire;

	FPlayerInputDelegate OnInputDelegate;
};
