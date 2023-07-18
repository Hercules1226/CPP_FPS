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

	// 내 액터찾기
	me = Cast<AEnemy>(GetOwner());

	// 	   -> 에디터에서 할당해주기
	// 	   -> 동적으로 타겟을 찾아야 할 필요
	target = Cast<AFPSPlayer>(UGameplayStatics::GetActorOfClass(GetWorld(), AFPSPlayer::StaticClass()));

	// AIController 할당
	ai = Cast<AAIController>(me->GetController());

	anim = Cast<UEnemyAnimInstance>(me->GetMesh()->GetAnimInstance());

	// 사용할 ns 할당
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

	// 목차(뼈대)를 작성
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

// 일정시간동안 기다리다가 상태를 Move 로 바꿔주자.
// 필요속성 : 기다릴시간, 경과시간
void UEnemyFSM::IdleState()
{
	// 일정시간동안 기다리다가 상태를 Move 로 바꿔주자.
	// 1. 시간이 흘렀으니까
	currentTime += GetWorld()->DeltaTimeSeconds;
	// 2. 대기시간이 다됐으니까
	if(currentTime > idleDelayTime)
	{
		// 3. 상태를 Patrol 로 바꿔주자.
		m_state = EEnemyState::Patrol;
		// 4. Animation 의 상태도 Patrol 로 바꿔주고 싶다.
		anim->state = m_state;

		me->GetCharacterMovement()->MaxWalkSpeed = 200;
		GetTargetLocation(me, 1000, randomPos);

		// -> 속도가 있을 때 move 로 바꿔주자
		currentTime = 0;
	}
}

// 랜덤한 위치를 찾아서 돌아다닌다.
// 단, 플레이어와의 거리가 일정 범위안에 들어오면 상태를 Move 로 바꾼다.
void UEnemyFSM::PatrolState()
{
	// AI의 길찾기 기능을 이용해서 이동하고 싶다.
	// 타겟과의 거리가 1000 이내로 들어오고 움직일 수 있는 환경이라면 상태를 Move 로 전환
	float distance = FVector::Dist(target->GetActorLocation(), me->GetActorLocation());
	// 단, 플레이어와의 거리가 일정 범위안에 들어오면 상태를 Move 로 바꾼다.
	if (distance < 1000 && CanMove())
	{
		m_state = EEnemyState::Move;
		me->GetCharacterMovement()->MaxWalkSpeed = 400;

		anim->state = m_state;
		return;
	}

	EPathFollowingRequestResult::Type result = ai->MoveToLocation(randomPos, attackRange);
	// 도착했다면 다시 랜덤한 위치 설정
	if (result == EPathFollowingRequestResult::AlreadyAtGoal)
	{
		GetTargetLocation(me, 1000, randomPos);
	}

	//aiDebugActor->SetActorLocation(randomPos);

}

// 1. 타겟 방향으로 이동하고 싶다.
// 필요속성 : 타겟, 이동속도
// 2. 타겟과의 거리가 공격범위안에 들어오면 상태를 공격으로 바꾸고 싶다.
// 필요속성 : 공격범위
void UEnemyFSM::MoveState()
{
	CanMove();

	// 만약 애니메이션이 Attack 을 플레이하고 있으면???
	// 아래 내용은 진행하지 않는다.
	int32 index = anim->GetStateMachineIndex(TEXT("FSM"));
	FAnimNode_StateMachine* sm = anim->GetStateMachineInstance(index);
	if (sm->GetCurrentStateName() == TEXT("Attack"))
	{
		return;
	}

	// 타겟 방향으로 이동하고 싶다.
	EPathFollowingRequestResult::Type r = ai->MoveToActor(target);

	// 타겟 방향으로 이동이 불가능하면 상태를 Patrol 로 바꿔주자
	if(r == EPathFollowingRequestResult::Failed)
	{
		// -> 상태를 다시 Patrol 바꿔주자
		m_state = EEnemyState::Patrol;
		GetTargetLocation(me, 1000, randomPos);
		// -> Patrol 일때 속는 200 정도로 하자
		me->GetCharacterMovement()->MaxWalkSpeed = 200;

		anim->state = m_state;
		return;
	}
	
	// 공격범위를 시각적으로 표현해보자
	DrawDebugSphere(GetWorld(), me->GetActorLocation(), attackRange, 10, FColor::Red);

	// 2. 타겟과의 거리가 공격범위안에 들어오면 상태를 공격으로 바꾸고 싶다.
	float distance = FVector::Dist(target->GetActorLocation(), me->GetActorLocation());

	if (distance < attackRange)
	{
		m_state = EEnemyState::AttackDelay;
		currentTime = attackDelayTime;
		anim->state = m_state;

		// AI 길찾기 꺼주자
		ai->StopMovement();
	}
}

// 일정시간에 한번씩 공격하고 싶다.
// 필요속성 : 공격대기시간
void UEnemyFSM::AttackState()
{
	m_state = EEnemyState::AttackDelay;
	anim->state = m_state;
	// 일정시간에 한번씩 공격하고 싶다.	
	// 1. 시간이 흘렀으니까
	currentTime += GetWorld()->DeltaTimeSeconds;
	// 2. 공격시간이 됐으니까
	if(currentTime > attackDelayTime)
	{
		// 3. 공격을 콘솔에 출력
		PRINTLOG(TEXT("Attack!!!"));
		currentTime = 0;
		m_state = EEnemyState::Attack;
		anim->state = m_state;

	}
	
	// 공격 상대를 바라보고 싶다.
	FVector direction = target->GetActorLocation() - me->GetActorLocation();
	direction.Normalize();
	me->SetActorRotation(direction.ToOrientationRotator());

	// 타겟이 도망가면 따라가고 싶다.
	// 타겟과의 거리
	//FVector direction = target->GetActorLocation() - me->GetActorLocation();
	// 둘사이의 거리
	float distance = FVector::Dist(target->GetActorLocation(), me->GetActorLocation());
	// -> 상태를 Move 로 전환하고 싶다.
	// -> 타겟과의 거리가 공격범위를 벗어나면
	if (distance > attackRange)
	{
		m_state = EEnemyState::Move;
		anim->state = m_state;
	}
}

// 일정시간이 지나면 상태를 Idle 로 바꾸고 싶다.
// 필요속성 : 피격대기시간
// -> 넉백이 끝나면 Idle 로 상태를 바꾸자.
void UEnemyFSM::DamageState()
{
	// Lerp 를 이용하여 knock back 구현
	FVector myPos = me->GetActorLocation();

	myPos = FMath::Lerp(myPos, knockbackPos, 20 * GetWorld()->DeltaTimeSeconds);

	float distance = FVector::Dist(myPos, knockbackPos);
	// 최종 위치와 나와의 거리가 아주 작으면 도착한것으로 판단하자
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

// AI 가 이동할 때 최종 목적지(target) 에 갈 수 있는지 확인
// -> AI 가 이동할 경로 데이터가 필요
// -> 마지막 두점 위치를 표시해보자
// -> 마지막 경로 위치에서 target 쪽으로 LineTrace 쏴서 충돌체크
// -> target 하고 충돌이 발생하면 : 이동 가능하다
// -> 이동할 경로 시각화 (서비스)
bool UEnemyFSM::CanMove()
{
	// -> AI 가 이동할 경로 데이터가 필요
	// 1. 이동경로 데이터 가져오기
	FPathFindingQuery query;
	FAIMoveRequest req;
	req.SetAcceptanceRadius(3);
	req.SetGoalActor(target);
	ai->BuildPathfindingQuery(req ,query);
	FPathFindingResult result = ns->FindPathSync(query);

	TArray<FNavPathPoint> points = result.Path->GetPathPoints();
	int32 num = points.Num();

	PRINTLOG(TEXT("Path Num : %d"), num);

	// -> 마지막 (경로들)두점 위치를 표시해보자
	// 1. 경로가 하나라도 있을 때
	//if(num > 1)
	//{
	//	for(int i=0;i<pathActors.Num();i++)
	//	{
	//		// 2. 몇번째 경로인지 알아야한다.
	//		// 배열의 시작은 0부터 시작한다. 크기가 n 일때 마지막 원소는 n - 1이된다.
	//		int32 index = num - (i + 1);
	//		// path 갯수보다 등록된 pathActors 의 갯수가 더 많을 때
	//		if (index < 0)
	//		{
	//			break;
	//		}

	//		// 3. 경로의 위치에 물체를 배치하자
	//		pathActors[i]->SetActorLocation(points[index].Location);
	//	}
	//}

	// -> 이동할 경로 시각화 (서비스) : 선을 그려보자
	for (int i = 1;i<num;i++)
	{
		FVector point1 = points[i - 1].Location;
		FVector point2 = points[i].Location;

		DrawDebugLine(GetWorld(), point1, point2, FColor::Red, false, 0.1f, 10, 5);
	}




	// -> 마지막 경로 위치에서 target 쪽으로 LineTrace 쏴서 충돌체크
	// -> target 하고 충돌이 발생하면 : 이동 가능하다
	if (num > 0)
	{
		// -> 마지막 경로 위치에서 target 쪽으로 LineTrace 쏴서 충돌체크
		// 1. 시작점 필요
		FVector startPoint = points[num - 1].Location;
		// 2. 끝점이 필요
		FVector endPoint = target->GetActorLocation();
		// 3. 나(Enemy)는 충돌 무시해라
		FCollisionQueryParams param;
		param.AddIgnoredActor(me);
		// 4. LineTrace 쏴서 충돌체크
		FHitResult hitInfo;
		bool r = GetWorld()->LineTraceSingleByChannel(hitInfo, startPoint, endPoint, ECC_WorldStatic, param);

		// -> target 하고 충돌이 발생하면 : 이동 가능하다 true 반환
		if (r && hitInfo.GetActor() == target)
		{
			return true;
		}
	}


	return false;
}

// 피격 받았을 때 처리할 함수
// 피격 받았을 때 hp 를 감소시키고 0 이하면 상태를 Die 로 바꾸고 없애버리자
void UEnemyFSM::OnDamageProcess(FVector shootDirection)
{
	// 만약 hp 가 0 이하면 => Die
	// 아래 내용은 수행하지 않는다.
	if (m_state == EEnemyState::Die)
	{
		return;
	}


	ai->StopMovement();
	// 맞았을 때 상대를 바라보도록 하자
	me->SetActorRotation((-shootDirection).ToOrientationRotator());

	hp--;
	if (hp <= 0)
	{
		m_state = EEnemyState::Die;
		anim->Die();
		// 충돌체는 다 꺼주자
		me->GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		me->GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		return;
	}
	
	// 맞으면 뒤로 밀리도록 처리하고 싶다.
	// 밀릴 방향이 필요
	//me->SetActorLocation(me->GetActorLocation() + shootDirection * knockback);
	shootDirection.Z = 0;
	knockbackPos = me->GetActorLocation() + shootDirection * knockback;

	// 상태를 Damage 로 바꾸고 싶다.
	m_state = EEnemyState::Damage;

	anim->Hit();

	// 알람맞춰놓고 시간이 다되면 상태를 Idle 로 바꾸고 싶다.
	FTimerHandle damageTimer;

	//GetWorld()->GetTimerManager().SetTimer(damageTimer, this, &UEnemyFSM::DamageState, damageDelayTime, false);
}

