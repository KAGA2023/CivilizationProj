// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToHex.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "../UnitAIController.h"
#include "../../Unit/UnitCharacterBase.h"
#include "../../World/WorldComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_MoveToHex::UBTTask_MoveToHex()
{
    NodeName = TEXT("Move To Hex");
    BBKey_TargetHexPosition.SelectedKeyName = TEXT("TargetHexPosition");
    
    // Task가 틱을 사용하도록 설정
    bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_MoveToHex::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    APawn* Pawn = AIController->GetPawn();
    if (!Pawn)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard)
    {
        return EBTNodeResult::Failed;
    }

    // Blackboard에서 목표 헥스 좌표 가져오기
    FVector TargetHexVector = Blackboard->GetValueAsVector(BBKey_TargetHexPosition.SelectedKeyName);
    FVector2D TargetHex(TargetHexVector.X, TargetHexVector.Y);

    // 유효하지 않은 헥스 좌표 확인
    if (TargetHex.X == 0.0f && TargetHex.Y == 0.0f)
    {
        return EBTNodeResult::Failed;
    }

    // 헥스 좌표를 월드 좌표로 변환 (같은 높이만 고려)
    FVector TargetWorldPosition = GetWorldPositionFromHex(TargetHex, UnitAIController);
    
    if (TargetWorldPosition == FVector::ZeroVector)
    {
        return EBTNodeResult::Failed;
    }

    // 현재 위치와 목표 위치의 거리 확인
    FVector CurrentPosition = Pawn->GetActorLocation();
    FVector Direction = TargetWorldPosition - CurrentPosition;
    Direction.Z = 0.0f; // Z축은 무시
    
    float Distance = Direction.Size();
    
    // 이미 도착했는지 확인
    if (Distance <= AcceptanceRadius)
    {
        // 다음 헥스로 이동
        UnitAIController->MoveToNextHexInPath();
        
        // 경로가 끝났는지 확인
        TArray<FVector2D> CurrentPath = UnitAIController->GetCurrentMovementPath();
        int32 CurrentIndex = UnitAIController->GetCurrentPathIndex();
        
        if (CurrentIndex >= CurrentPath.Num())
        {
            // 이동 완료
            UnitAIController->CompleteMovement();
            return EBTNodeResult::Succeeded;
        }
        
        // 다음 헥스가 있으면 Blackboard 업데이트 후 다시 이동
        FVector2D NextHex = CurrentPath[CurrentIndex];
        Blackboard->SetValueAsVector(BBKey_TargetHexPosition.SelectedKeyName, FVector(NextHex.X, NextHex.Y, 0));
        
        // 다시 ExecuteTask 호출을 위해 InProgress 반환
        return EBTNodeResult::InProgress;
    }

    // CharacterMovementComponent를 사용한 직접 이동
    if (ACharacter* Character = Cast<ACharacter>(Pawn))
    {
        if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
        {
            // 이동 방향 계산
            FVector MoveDirection = Direction.GetSafeNormal();
            
            // 이동 입력 추가 (매 프레임 호출되므로 TickTask에서도 계속 호출)
            Character->AddMovementInput(MoveDirection, 1.0f);
            
            // 이동 중이므로 InProgress 반환
            return EBTNodeResult::InProgress;
        }
    }
    
    // 이동 실패
    return EBTNodeResult::Failed;
}

void UBTTask_MoveToHex::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

    APawn* Pawn = AIController->GetPawn();
    if (!Pawn)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (!Blackboard)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // Blackboard에서 목표 헥스 좌표 가져오기
    FVector TargetHexVector = Blackboard->GetValueAsVector(BBKey_TargetHexPosition.SelectedKeyName);
    FVector2D TargetHex(TargetHexVector.X, TargetHexVector.Y);

    // 헥스 좌표를 월드 좌표로 변환
    FVector TargetWorldPosition = GetWorldPositionFromHex(TargetHex, UnitAIController);
    
    if (TargetWorldPosition == FVector::ZeroVector)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 현재 위치와 목표 위치의 거리 확인 (XY 평면만)
    FVector CurrentPosition = Pawn->GetActorLocation();
    FVector Direction = TargetWorldPosition - CurrentPosition;
    Direction.Z = 0.0f; // Z축은 무시
    
    float Distance = Direction.Size();

    // 도착 확인
    if (Distance <= AcceptanceRadius)
    {
        // 다음 헥스로 이동
        UnitAIController->MoveToNextHexInPath();
        
        // 경로가 끝났는지 확인
        TArray<FVector2D> CurrentPath = UnitAIController->GetCurrentMovementPath();
        int32 CurrentIndex = UnitAIController->GetCurrentPathIndex();
        
        if (CurrentIndex >= CurrentPath.Num())
        {
            // 이동 완료
            UnitAIController->CompleteMovement();
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            return;
        }
        
        // 다음 헥스가 있으면 Blackboard 업데이트 후 계속 이동
        FVector2D NextHex = CurrentPath[CurrentIndex];
        Blackboard->SetValueAsVector(BBKey_TargetHexPosition.SelectedKeyName, FVector(NextHex.X, NextHex.Y, 0));
        
        // 다음 헥스로 이동 계속
        FVector NextWorldPosition = GetWorldPositionFromHex(NextHex, UnitAIController);
        FVector NextDirection = (NextWorldPosition - CurrentPosition).GetSafeNormal();
        NextDirection.Z = 0.0f;
        
        if (ACharacter* Character = Cast<ACharacter>(Pawn))
        {
            Character->AddMovementInput(NextDirection, 1.0f);
        }
    }
    else
    {
        // 아직 도착하지 않았으면 계속 이동
        if (ACharacter* Character = Cast<ACharacter>(Pawn))
        {
            FVector MoveDirection = Direction.GetSafeNormal();
            Character->AddMovementInput(MoveDirection, 1.0f);
        }
    }
}

void UBTTask_MoveToHex::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

FVector UBTTask_MoveToHex::GetWorldPositionFromHex(FVector2D HexPosition, AUnitAIController* AIController) const
{
    if (!AIController)
    {
        return FVector::ZeroVector;
    }

    // AIController의 HexToWorld 함수 사용
    FVector WorldPosition = AIController->HexToWorld(HexPosition);

    // 같은 높이만 고려: 현재 유닛의 높이를 유지
    // (나중에 높이 차이 처리 추가 예정)
    APawn* Pawn = AIController->GetPawn();
    if (Pawn)
    {
        // 현재 유닛의 Z 좌표를 유지 (같은 높이 이동)
        WorldPosition.Z = Pawn->GetActorLocation().Z;
    }
    else
    {
        // Pawn이 없으면 기본 높이(평지) 사용
        WorldPosition.Z = 183.0f;
    }

    return WorldPosition;
}
