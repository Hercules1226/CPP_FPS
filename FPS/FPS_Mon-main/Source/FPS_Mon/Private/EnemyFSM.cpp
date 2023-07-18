// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyFSM.h"
#include "FPSPlayer.h"
#include <Kismet/GameplayStatics.h>
#include <EngineUtils.h>
#include "Enemy.h"
#include <DrawDebugHelpers.h>
#include "FPS_Mon.h"
#include <AIController.h>
#include "EnemyAnimInstance.h"
#include <NavigationSystem.h>
#include <GameFramework/CharacterMovementComponent.h>
#include <NavigationInvokerComponent.h>
#include <Animation/AnimNode_StateMachine.h>
#include <Components/CapsuleComponent.h>

// Sets default values for this component's properties
UEnemyFSM::UEnemyFSM()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UEnemyFSM::BeginPlay()
{
	Super::BeginPlay();

	// �� ����ã��
	me = Cast<AEnemy>(GetOwner());

	// 	   -> �����Ϳ��� �Ҵ����ֱ�
	// 	   -> �������� Ÿ���� ã�ƾ� �� �ʿ�
	target = Cast<AFPSPlayer>(UGameplayStatics::GetActorOfClass(GetWorld(), AFPSPlayer::StaticClass()));

	// AIController �Ҵ�
	ai = Cast<AAIController>(me->GetController());

	anim = Cast<UEnemyAnimInstance>(me->GetMesh()->GetAnimInstance());

	// ����� ns �Ҵ�
	ns = UNavigationSystemV1::GetNavigationSystem(GetWorld());


	anim->state = m_state;

	/*TArray<AActor*> actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AFPSPlayer::StaticClass(), actors);*/

	/*for (int i=0;i<actors.Num();i++)
	{
		AActor* t = actors[i];
		target = Cast<AFPSPlayer>(t);

	}*/

	/*for (auto t : actors)
	{
		target = Cast<AFPSPlayer>(t);
		break;
	}*/

	/*for (TActorIterator<AFPSPlayer> it(GetWorld()); it; ++it)
	{
		target = *it;
	}*/
	
}


// Called every frame
void UEnemyFSM::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ����(����)�� �ۼ�
	switch (m_state)
	{
	case EEnemyState::Idle:
		IdleState();
		break;
	case EEnemyState::Patrol:
		PatrolState();
		break;
	case EEnemyState::Move:
		MoveState();
		break;
	case EEnemyState::AttackDelay:
	case EEnemyState::Attack:
		AttackState();
		break;
	case EEnemyState::Damage:
		DamageState();
		break;
	case EEnemyState::Die:
		DieState();
		break;
	}
}

// �����ð����� ��ٸ��ٰ� ���¸� Move �� �ٲ�����.
// �ʿ�Ӽ� : ��ٸ��ð�, ����ð�
void UEnemyFSM::IdleState()
{
	// �����ð����� ��ٸ��ٰ� ���¸� Move �� �ٲ�����.
	// 1. �ð��� �귶���ϱ�
	currentTime += GetWorld()->DeltaTimeSeconds;
	// 2. ���ð��� �ٵ����ϱ�
	if(currentTime > idleDelayTime)
	{
		// 3. ���¸� Patrol �� �ٲ�����.
		m_state = EEnemyState::Patrol;
		// 4. Animation �� ���µ� Patrol �� �ٲ��ְ� �ʹ�.
		anim->state = m_state;

		me->GetCharacterMovement()->MaxWalkSpeed = 200;
		GetTargetLocation(me, 1000, randomPos);

		// -> �ӵ��� ���� �� move �� �ٲ�����
		currentTime = 0;
	}
}

// ������ ��ġ�� ã�Ƽ� ���ƴٴѴ�.
// ��, �÷��̾���� �Ÿ��� ���� �����ȿ� ������ ���¸� Move �� �ٲ۴�.
void UEnemyFSM::PatrolState()
{
	// AI�� ��ã�� ����� �̿��ؼ� �̵��ϰ� �ʹ�.
	// Ÿ�ٰ��� �Ÿ��� 1000 �̳��� ������ ������ �� �ִ� ȯ���̶�� ���¸� Move �� ��ȯ
	float distance = FVector::Dist(target->GetActorLocation(), me->GetActorLocation());
	// ��, �÷��̾���� �Ÿ��� ���� �����ȿ� ������ ���¸� Move �� �ٲ۴�.
	if (distance < 1000 && CanMove())
	{
		m_state = EEnemyState::Move;
		me->GetCharacterMovement()->MaxWalkSpeed = 400;

		anim->state = m_state;
		return;
	}

	EPathFollowingRequestResult::Type result = ai->MoveToLocation(randomPos, attackRange);
	// �����ߴٸ� �ٽ� ������ ��ġ ����
	if (result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		GetTargetLocation(me, 1000, randomPos);
	}

	//aiDebugActor->SetActorLocation(randomPos);

}

// 1. Ÿ�� �������� �̵��ϰ� �ʹ�.
// �ʿ�Ӽ� : Ÿ��, �̵��ӵ�
// 2. Ÿ�ٰ��� �Ÿ��� ���ݹ����ȿ� ������ ���¸� �������� �ٲٰ� �ʹ�.
// �ʿ�Ӽ� : ���ݹ���
void UEnemyFSM::MoveState()
{
	CanMove();

	// ���� �ִϸ��̼��� Attack �� �÷����ϰ� ������???
	// �Ʒ� ������ �������� �ʴ´�.
	int32 index = anim->GetStateMachineIndex(TEXT("FSM"));
	FAnimNode_StateMachine* sm = anim->GetStateMachineInstance(index);
	if (sm->GetCurrentStateName() == TEXT("Attack"))
	{
		return;
	}

	// Ÿ�� �������� �̵��ϰ� �ʹ�.
	EPathFollowingRequestResult::Type r = ai->MoveToActor(target);

	// Ÿ�� �������� �̵��� �Ұ����ϸ� ���¸� Patrol �� �ٲ�����
	if(r == EPathFollowingRequestResult::Failed)
	{
		// -> ���¸� �ٽ� Patrol �ٲ�����
		m_state = EEnemyState::Patrol;
		GetTargetLocation(me, 1000, randomPos);
		// -> Patrol �϶� �Ӵ� 200 ������ ����
		me->GetCharacterMovement()->MaxWalkSpeed = 200;

		anim->state = m_state;
		return;
	}
	
	// ���ݹ����� �ð������� ǥ���غ���
	DrawDebugSphere(GetWorld(), me->GetActorLocation(), attackRange, 10, FColor::Red);

	// 2. Ÿ�ٰ��� �Ÿ��� ���ݹ����ȿ� ������ ���¸� �������� �ٲٰ� �ʹ�.
	float distance = FVector::Dist(target->GetActorLocation(), me->GetActorLocation());

	if (distance < attackRange)
	{
		m_state = EEnemyState::AttackDelay;
		currentTime = attackDelayTime;
		anim->state = m_state;

		// AI ��ã�� ������
		ai->StopMovement();
	}
}

// �����ð��� �ѹ��� �����ϰ� �ʹ�.
// �ʿ�Ӽ� : ���ݴ��ð�
void UEnemyFSM::AttackState()
{
	m_state = EEnemyState::AttackDelay;
	anim->state = m_state;
	// �����ð��� �ѹ��� �����ϰ� �ʹ�.	
	// 1. �ð��� �귶���ϱ�
	currentTime += GetWorld()->DeltaTimeSeconds;
	// 2. ���ݽð��� �����ϱ�
	if(currentTime > attackDelayTime)
	{
		// 3. ������ �ֿܼ� ���
		PRINTLOG(TEXT("Attack!!!"));
		currentTime = 0;
		m_state = EEnemyState::Attack;
		anim->state = m_state;

	}
	
	// ���� ��븦 �ٶ󺸰� �ʹ�.
	FVector direction = target->GetActorLocation() - me->GetActorLocation();
	direction.Normalize();
	me->SetActorRotation(direction.ToOrientationRotator());

	// Ÿ���� �������� ���󰡰� �ʹ�.
	// Ÿ�ٰ��� �Ÿ�
	//FVector direction = target->GetActorLocation() - me->GetActorLocation();
	// �ѻ����� �Ÿ�
	float distance = FVector::Dist(target->GetActorLocation(), me->GetActorLocation());
	// -> ���¸� Move �� ��ȯ�ϰ� �ʹ�.
	// -> Ÿ�ٰ��� �Ÿ��� ���ݹ����� �����
	if (distance > attackRange)
	{
		m_state = EEnemyState::Move;
		anim->state = m_state;
	}
}

// �����ð��� ������ ���¸� Idle �� �ٲٰ� �ʹ�.
// �ʿ�Ӽ� : �ǰݴ��ð�
// -> �˹��� ������ Idle �� ���¸� �ٲ���.
void UEnemyFSM::DamageState()
{
	// Lerp �� �̿��Ͽ� knock back ����
	FVector myPos = me->GetActorLocation();

	myPos = FMath::Lerp(myPos, knockbackPos, 20 * GetWorld()->DeltaTimeSeconds);

	float distance = FVector::Dist(myPos, knockbackPos);
	// ���� ��ġ�� ������ �Ÿ��� ���� ������ �����Ѱ����� �Ǵ�����
	if (distance < 21.0f)
	{
		myPos = knockbackPos;
		m_state = EEnemyState::Idle;
		currentTime = 0;
	}

	me->SetActorLocation(myPos);
}

void UEnemyFSM::DieState()
{
}

bool UEnemyFSM::GetTargetLocation(const AActor* targetActor, float radius, FVector& dest)
{
	FNavLocation loc;
	bool result = ns->GetRandomReachablePointInRadius(targetActor->GetActorLocation(), radius, loc);
	dest = loc.Location;
	return result;
}

// AI �� �̵��� �� ���� ������(target) �� �� �� �ִ��� Ȯ��
// -> AI �� �̵��� ��� �����Ͱ� �ʿ�
// -> ������ ���� ��ġ�� ǥ���غ���
// -> ������ ��� ��ġ���� target ������ LineTrace ���� �浹üũ
// -> target �ϰ� �浹�� �߻��ϸ� : �̵� �����ϴ�
// -> �̵��� ��� �ð�ȭ (����)
bool UEnemyFSM::CanMove()
{
	// -> AI �� �̵��� ��� �����Ͱ� �ʿ�
	// 1. �̵���� ������ ��������
	FPathFindingQuery query;
	FAIMoveRequest req;
	req.SetAcceptanceRadius(3);
	req.SetGoalActor(target);
	ai->BuildPathfindingQuery(req ,query);
	FPathFindingResult result = ns->FindPathSync(query);

	TArray<FNavPathPoint> points = result.Path->GetPathPoints();
	int32 num = points.Num();

	PRINTLOG(TEXT("Path Num : %d"), num);

	// -> ������ (��ε�)���� ��ġ�� ǥ���غ���
	// 1. ��ΰ� �ϳ��� ���� ��
	//if(num > 1)
	//{
	//	for(int i=0;i<pathActors.Num();i++)
	//	{
	//		// 2. ���° ������� �˾ƾ��Ѵ�.
	//		// �迭�� ������ 0���� �����Ѵ�. ũ�Ⱑ n �϶� ������ ���Ҵ� n - 1�̵ȴ�.
	//		int32 index = num - (i + 1);
	//		// path �������� ��ϵ� pathActors �� ������ �� ���� ��
	//		if (index < 0)
	//		{
	//			break;
	//		}

	//		// 3. ����� ��ġ�� ��ü�� ��ġ����
	//		pathActors[i]->SetActorLocation(points[index].Location);
	//	}
	//}

	// -> �̵��� ��� �ð�ȭ (����) : ���� �׷�����
	for (int i = 1;i<num;i++)
	{
		FVector point1 = points[i - 1].Location;
		FVector point2 = points[i].Location;

		DrawDebugLine(GetWorld(), point1, point2, FColor::Red, false, 0.1f, 10, 5);
	}




	// -> ������ ��� ��ġ���� target ������ LineTrace ���� �浹üũ
	// -> target �ϰ� �浹�� �߻��ϸ� : �̵� �����ϴ�
	if (num > 0)
	{
		// -> ������ ��� ��ġ���� target ������ LineTrace ���� �浹üũ
		// 1. ������ �ʿ�
		FVector startPoint = points[num - 1].Location;
		// 2. ������ �ʿ�
		FVector endPoint = target->GetActorLocation();
		// 3. ��(Enemy)�� �浹 �����ض�
		FCollisionQueryParams param;
		param.AddIgnoredActor(me);
		// 4. LineTrace ���� �浹üũ
		FHitResult hitInfo;
		bool r = GetWorld()->LineTraceSingleByChannel(hitInfo, startPoint, endPoint, ECC_WorldStatic, param);

		// -> target �ϰ� �浹�� �߻��ϸ� : �̵� �����ϴ� true ��ȯ
		if (r && hitInfo.GetActor() == target)
		{
			return true;
		}
	}


	return false;
}

// �ǰ� �޾��� �� ó���� �Լ�
// �ǰ� �޾��� �� hp �� ���ҽ�Ű�� 0 ���ϸ� ���¸� Die �� �ٲٰ� ���ֹ�����
void UEnemyFSM::OnDamageProcess(FVector shootDirection)
{
	// ���� hp �� 0 ���ϸ� => Die
	// �Ʒ� ������ �������� �ʴ´�.
	if (m_state == EEnemyState::Die)
	{
		return;
	}


	ai->StopMovement();
	// �¾��� �� ��븦 �ٶ󺸵��� ����
	me->SetActorRotation((-shootDirection).ToOrientationRotator());

	hp--;
	if (hp <= 0)
	{
		m_state = EEnemyState::Die;
		anim->Die();
		// �浹ü�� �� ������
		me->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}
	
	// ������ �ڷ� �и����� ó���ϰ� �ʹ�.
	// �и� ������ �ʿ�
	//me->SetActorLocation(me->GetActorLocation() + shootDirection * knockback);
	shootDirection.Z = 0;
	knockbackPos = me->GetActorLocation() + shootDirection * knockback;

	// ���¸� Damage �� �ٲٰ� �ʹ�.
	m_state = EEnemyState::Damage;

	anim->Hit();

	// �˶�������� �ð��� �ٵǸ� ���¸� Idle �� �ٲٰ� �ʹ�.
	FTimerHandle damageTimer;

	//GetWorld()->GetTimerManager().SetTimer(damageTimer, this, &UEnemyFSM::DamageState, damageDelayTime, false);
}
