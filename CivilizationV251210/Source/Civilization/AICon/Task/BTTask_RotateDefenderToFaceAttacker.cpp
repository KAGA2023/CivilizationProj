// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_RotateDefenderToFaceAttacker.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "../UnitAIController.h"
#include "../../Unit/UnitCharacterBase.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_RotateDefenderToFaceAttacker::UBTTask_RotateDefenderToFaceAttacker()
{
    NodeName = TEXT("Rotate Defender To Face Attacker");
    
    // Task가 틱을 사용하도록 설정
    bCreateNodeInstance = false;
    bNotifyTick = true; // TickTask를 사용하기 위해 필수!
}

EBTNodeResult::Type UBTTask_RotateDefenderToFaceAttacker::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    AUnitAIController* UnitAIController = Cast<AUnitAIController>(AIController);
    if (!UnitAIController)
    {
        return EBTNodeResult::Failed;
    }

    // 방어자와 공격자 가져오기
    // 이 Task는 방어자의 AIController에서 실행되므로, Pawn이 방어자
    AUnitCharacterBase* Defender = Cast<AUnitCharacterBase>(AIController->GetPawn());
    AUnitCharacterBase* Attacker = UnitAIController->GetCombatAttacker();
    
    if (!Defender || !Attacker)
    {
        return EBTNodeResult::Failed;
    }

    // 공격자 위치로 회전 시작
    // 회전은 TickTask에서 처리하므로 InProgress 반환
    return EBTNodeResult::InProgress;
}

void UBTTask_RotateDefenderToFaceAttacker::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AUnitAIController* UnitAIController = Cast<AUnitAIController>(AIController);
    if (!UnitAIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 방어자와 공격자 가져오기
    AUnitCharacterBase* Defender = Cast<AUnitCharacterBase>(AIController->GetPawn());
    AUnitCharacterBase* Attacker = UnitAIController->GetCombatAttacker();
    
    if (!Defender || !Attacker)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 공격자 위치 가져오기
    FVector AttackerLocation = Attacker->GetActorLocation();
    FVector DefenderLocation = Defender->GetActorLocation();

    // 공격자 방향 계산 (Z축 무시)
    FVector Direction = AttackerLocation - DefenderLocation;
    Direction.Z = 0.0f;
    
    // 방향이 0이면 이미 정면을 보고 있는 것
    if (Direction.SizeSquared() < 0.01f)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    Direction.Normalize();

    // 목표 회전값 계산
    FRotator TargetRotation = Direction.Rotation();

    // 현재 회전값
    FRotator CurrentRotation = Defender->GetActorRotation();

    // 회전 보간 (Slerp 사용)
    FRotator NewRotation = FMath::RInterpTo(CurrentRotation, TargetRotation, DeltaSeconds, RotationSpeed / 90.0f);
    Defender->SetActorRotation(NewRotation);

    // 회전 완료 확인
    float YawDifference = FMath::Abs(FMath::FindDeltaAngleDegrees(CurrentRotation.Yaw, TargetRotation.Yaw));
    
    if (YawDifference <= RotationTolerance)
    {
        // 정확한 목표 회전값으로 설정
        Defender->SetActorRotation(TargetRotation);
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }
}

void UBTTask_RotateDefenderToFaceAttacker::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

