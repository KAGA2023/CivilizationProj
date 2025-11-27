// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveToHex.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "../UnitAIController.h"
#include "../../Unit/UnitCharacterBase.h"
#include "../../World/WorldComponent.h"
#include "../../World/WorldStruct.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_MoveToHex::UBTTask_MoveToHex()
{
    NodeName = TEXT("Move To Hex");
    BBKey_TargetHexPosition.SelectedKeyName = TEXT("TargetHexPosition");
    
    // Task가 틱을 사용하도록 설정
    bCreateNodeInstance = false;
    bNotifyTick = true; // TickTask를 사용하기 위해 필수!
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

    // 헥스 좌표를 월드 좌표로 변환
    FVector TargetWorldPosition = GetWorldPositionFromHex(TargetHex, UnitAIController);
    
    if (TargetWorldPosition == FVector::ZeroVector)
    {
        return EBTNodeResult::Failed;
    }

    // CharacterMovementComponent를 사용한 직접 이동 시작
    if (ACharacter* Character = Cast<ACharacter>(Pawn))
    {
        if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
        {
            // 이동 방향 계산
            FVector CurrentPosition = Pawn->GetActorLocation();
            FVector Direction = TargetWorldPosition - CurrentPosition;
            Direction.Z = 0.0f; // Z축은 무시 (같은 높이 이동)
            
            // 이동 입력 추가
            FVector MoveDirection = Direction.GetSafeNormal();
            Character->AddMovementInput(MoveDirection, 1.0f);
            
            // 이동 중이므로 InProgress 반환 (TickTask에서 도착 확인)
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
    Direction.Z = 0.0f; // Z축은 무시 (같은 높이 이동)
    
    float Distance = Direction.Size();

    // 점프 체크 (목표에 가까워졌을 때, 그리고 이 목표 타일에 대해 아직 점프하지 않았을 때)
    if (Distance <= 300.0f && LastJumpedTargetHex != TargetHex)
    {
        // 경로에서 현재 타일 위치 가져오기 (더 정확함)
        TArray<FVector2D> CurrentPath = UnitAIController->GetCurrentMovementPath();
        int32 CurrentIndex = UnitAIController->GetCurrentPathIndex();
        
        // 현재 타일 헥스 좌표 (경로에서 가져오기)
        FVector2D CurrentHex;
        if (CurrentPath.Num() > 0 && CurrentIndex > 0 && CurrentIndex <= CurrentPath.Num())
        {
            // 현재 인덱스는 다음 타일을 가리키므로, 이전 타일이 현재 타일
            CurrentHex = CurrentPath[CurrentIndex - 1];
        }
        else if (CurrentPath.Num() > 0)
        {
            // 인덱스가 0이거나 범위를 벗어난 경우 첫 번째 타일 사용
            CurrentHex = CurrentPath[0];
        }
        else
        {
            // 경로가 없으면 월드 좌표로 변환
            CurrentHex = UnitAIController->WorldToHex(CurrentPosition);
        }
        
        // WorldComponent에서 타일 정보 가져오기
        UWorldComponent* WorldComponent = UnitAIController->GetWorldComponent();
        if (WorldComponent)
        {
            UWorldTile* CurrentTile = WorldComponent->GetTileAtHex(CurrentHex);
            UWorldTile* TargetTile = WorldComponent->GetTileAtHex(TargetHex);
            
            if (CurrentTile && TargetTile)
            {
                ELandType CurrentLandType = CurrentTile->GetLandType();
                ELandType TargetLandType = TargetTile->GetLandType();
                
                // 점프 조건 체크:
                // 1. 평지 → 언덕
                // 2. 언덕 → 산
                bool bShouldJump = false;
                if (CurrentLandType == ELandType::Plains && TargetLandType == ELandType::Hills)
                {
                    bShouldJump = true;
                }
                else if (CurrentLandType == ELandType::Hills && TargetLandType == ELandType::Mountains)
                {
                    bShouldJump = true;
                }
                
                // 점프 실행
                if (bShouldJump)
                {
                    if (ACharacter* Character = Cast<ACharacter>(Pawn))
                    {
                        // CharacterMovementComponent를 통해 점프 가능 여부 확인
                        if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
                        {
                            // 점프 가능 여부 확인
                            // IsFalling()이 false이고 CanAttemptJump()가 true일 때만 점프
                            // 이미 점프 중이면 (IsFalling() == true) 다시 점프하지 않음
                            if (!MoveComp->IsFalling() && MoveComp->CanAttemptJump())
                            {
                                Character->Jump();
                                LastJumpedTargetHex = TargetHex; // 이 목표 타일에 대해 점프했음을 기록
                            }
                        }
                    }
                }
            }
        }
    }

    // 도착 확인
    if (Distance <= AcceptanceRadius)
    {
        // 경로가 끝났는지 먼저 확인 (다음 헥스로 이동하기 전에)
        TArray<FVector2D> CurrentPath = UnitAIController->GetCurrentMovementPath();
        int32 CurrentIndex = UnitAIController->GetCurrentPathIndex();
        
        // 마지막 타일에 도착했는지 확인
        if (CurrentIndex >= CurrentPath.Num() - 1)
        {
            // 마지막 타일에 도착했으므로 이동 멈춤 및 완료 처리
            if (ACharacter* Character = Cast<ACharacter>(Pawn))
            {
                if (UCharacterMovementComponent* MoveComp = Character->GetCharacterMovement())
                {
                    MoveComp->StopMovementImmediately();
                    
                    // 타일 중앙에 정확히 위치시키기
                    // 약간의 오차가 있어도 도착한 것으로 간주하고 정확한 위치로 강제 이동
                    FVector CorrectedPosition = TargetWorldPosition;
                    CorrectedPosition.Z = Pawn->GetActorLocation().Z; // 현재 높이 유지
                    Pawn->SetActorLocation(CorrectedPosition);
                }
            }
            
            // 이동 완료
            UnitAIController->CompleteMovement();
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            return;
        }
        
        // 마지막 타일이 아니면 멈추지 않고 다음 헥스로 이동
        // 다음 헥스로 이동 (인덱스 증가 및 Blackboard 업데이트)
        UnitAIController->MoveToNextHexInPath();
        
        // 새로운 목표 타일로 변경되므로 점프 플래그 리셋
        LastJumpedTargetHex = FVector2D::ZeroVector;
        
        // 다음 틱에서 새로운 목표로 이동 계속
        // (이번 틱에서는 현재 방향으로 계속 이동)
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

    // WorldComponent를 통해 타일 정보 가져오기
    UWorldComponent* WorldComponent = AIController->GetWorldComponent();
    if (WorldComponent)
    {
        UWorldTile* Tile = WorldComponent->GetTileAtHex(HexPosition);
        if (Tile)
        {
            // 타일의 LandType에 따라 올바른 높이 설정
            ELandType LandType = Tile->GetLandType();
            switch (LandType)
            {
            case ELandType::Plains:
                WorldPosition.Z = 183.0f; // 평지
                break;
            case ELandType::Hills:
                WorldPosition.Z = 256.0f; // 언덕
                break;
            case ELandType::Mountains:
                WorldPosition.Z = 329.0f; // 산
                break;
            default:
                WorldPosition.Z = 183.0f; // 기본값 (평지)
                break;
            }
        }
        else
        {
            // 타일이 없으면 기본 높이(평지) 사용
            WorldPosition.Z = 183.0f;
        }
    }
    else
    {
        // WorldComponent가 없으면 기본 높이(평지) 사용
        WorldPosition.Z = 183.0f;
    }

    return WorldPosition;
}
