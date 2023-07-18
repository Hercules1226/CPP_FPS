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

	// �ǰ� ������ �� ȣ��� �Լ�(Animation ���)
	void Hit();
	// �׾��� �� ȣ��� �Լ�
	void Die();

	UFUNCTION(BlueprintCallable, Category = FSM)
	void OnNotifyTest();

	UFUNCTION()
	void OnDieMontageEnded(UAnimMontage* animMontage, bool isEnd);
};