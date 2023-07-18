// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PlayerMove.generated.h"

// 사용자의 입력을 받아 이동
// 필요속성:이동속도, 소유액터

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FPS_MON_API UPlayerMove : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPlayerMove();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void InitializeComponent() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent);

	void Horizontal(float value);
	void Vertical(float value);
	// 회전
	void Turn(float value);
	void LookUp(float value);
	// 점프
	void Jump();

public:
	// 필요속성:소유액터
	UPROPERTY()
	class AFPSPlayer* me;
};
