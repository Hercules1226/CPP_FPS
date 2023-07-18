// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "EnemyFSM.h"
#include "EnemyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class FPS_MON_API UEnemyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category=FSM, BlueprintReadOnly)
	bool isMoving = false;

	UPROPERTY(EditAnywhere, Category = FSM, BlueprintReadOnly)
	bool isPatrol = false;

	UPROPERTY(EditAnywhere, Category = FSM, BlueprintReadOnly)
	EEnemyState state;

	UPROPERTY(EditAnywhere, Category = FSM, BlueprintReadOnly)
	class UAnimMontage* damageMontage;

	UPROPERTY(EditAnywhere, Category = FSM, BlueprintReadOnly)
	class UAnimMontage* dieMontage;

	// 피격 당했을 때 호출될 함수(Animation 재생)
	void Hit();
	// 죽었을 때 호출될 함수
	void Die();

	UFUNCTION(BlueprintCallable, Category = FSM)
	void OnNotifyTest();

	UFUNCTION()
	void OnDieMontageEnded(UAnimMontage* animMontage, bool isEnd);
};
