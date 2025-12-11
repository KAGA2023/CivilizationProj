// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_MoveAttackerToDefenderTile.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "../UnitAIController.h"
#include "../../Unit/UnitCharacterBase.h"
#include "../../World/WorldComponent.h"
#include "../../World/WorldStruct.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UBTTask_MoveAttackerToDefenderTile::UBTTask_MoveAttackerToDefenderTile()
{
    NodeName = TEXT("Move Attacker To Defender Tile");
    
    // Task가 틱을 사용하도록 설정
    bCreateNodeInstance = false;
    bNotifyTick = true; // TickTask를 사용하기 위해 필수!
}

EBTNodeResult::Type UBTTask_MoveAttackerToDefenderTile::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // 공격자와 방어자 가져오기
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(Pawn);
    AUnitCharacterBase* Defender = UnitAIController->GetCombatDefender();
    
    if (!Attacker || !Defender)
    {
        return EBTNodeResult::Failed;
    }

    // 방어자 타일의 공격자 배치 위치 계산 (방어자와 겹치지 않는 위치)
    FVector TargetWorldPosition = CalculateAttackerPositionOnDefenderTile(UnitAIController, Defender);
    
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

void UBTTask_MoveAttackerToDefenderTile::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

    // 공격자와 방어자 가져오기
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(Pawn);
    AUnitCharacterBase* Defender = UnitAIController->GetCombatDefender();
    
    if (!Attacker || !Defender)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 방어자 타일의 공격자 배치 위치 계산
    FVector TargetWorldPosition = CalculateAttackerPositionOnDefenderTile(UnitAIController, Defender);
    
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

    // 방어자 타일의 헥스 좌표 가져오기
    UWorldComponent* WorldComponent = UnitAIController->GetWorldComponent();
    if (!WorldComponent)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    FVector2D DefenderHex = WorldComponent->WorldToHex(Defender->GetActorLocation());
    FVector2D AttackerHex = WorldComponent->WorldToHex(CurrentPosition);

    // 점프 체크 (목표에 가까워졌을 때, 그리고 이 목표 타일에 대해 아직 점프하지 않았을 때)
    if (Distance <= 300.0f && LastJumpedTargetHex != DefenderHex)
    {
        // WorldComponent에서 타일 정보 가져오기
        UWorldTile* AttackerTile = WorldComponent->GetTileAtHex(AttackerHex);
        UWorldTile* DefenderTile = WorldComponent->GetTileAtHex(DefenderHex);
        
        if (AttackerTile && DefenderTile)
        {
            ELandType AttackerLandType = AttackerTile->GetLandType();
            ELandType DefenderLandType = DefenderTile->GetLandType();
            
            // 점프 조건 체크:
            // 1. 평지 → 언덕
            // 2. 언덕 → 산
            bool bShouldJump = false;
            if (AttackerLandType == ELandType::Plains && DefenderLandType == ELandType::Hills)
            {
                bShouldJump = true;
            }
            else if (AttackerLandType == ELandType::Hills && DefenderLandType == ELandType::Mountains)
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
                            LastJumpedTargetHex = DefenderHex; // 이 목표 타일에 대해 점프했음을 기록
                        }
                    }
                }
            }
        }
    }

    // 도착 확인
    if (Distance <= AcceptanceRadius)
    {
        // 도착했으므로 이동 멈춤 및 완료 처리
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
        
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
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

void UBTTask_MoveAttackerToDefenderTile::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

FVector UBTTask_MoveAttackerToDefenderTile::CalculateAttackerPositionOnDefenderTile(AUnitAIController* AIController, AUnitCharacterBase* Defender) const
{
    if (!AIController || !Defender)
    {
        return FVector::ZeroVector;
    }

    UWorldComponent* WorldComponent = AIController->GetWorldComponent();
    if (!WorldComponent)
    {
        return FVector::ZeroVector;
    }

    // 방어자 위치와 타일 정보 가져오기
    FVector DefenderWorldPos = Defender->GetActorLocation();
    FVector2D DefenderHex = WorldComponent->WorldToHex(DefenderWorldPos);
    
    // 방어자 타일의 정중앙 위치 계산
    FVector DefenderTileCenter = GetWorldPositionFromHex(DefenderHex, AIController);
    
    // 공격자 위치 계산 (방어자 타일의 정중앙이 아닌, 방어자와 겹치지 않는 위치)
    // 방어자 위치에서 공격자 위치로의 방향 벡터 계산
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(AIController->GetPawn());
    if (!Attacker)
    {
        return FVector::ZeroVector;
    }

    FVector AttackerCurrentPos = Attacker->GetActorLocation();
    FVector ToDefender = DefenderWorldPos - AttackerCurrentPos;
    ToDefender.Z = 0.0f; // Z축은 무시
    ToDefender.Normalize();

    // 방어자 타일의 정중앙에서 방어자 방향으로 약간 뒤로 이동한 위치
    // 타일 경계 근처에 배치 (방어자와 겹치지 않도록)
    float OffsetDistance = 120.0f; // 방어자로부터 떨어진 거리 (타일 크기에 따라 조정 가능)
    FVector AttackerPosition = DefenderTileCenter - (ToDefender * OffsetDistance);
    
    // 높이는 방어자 타일의 높이와 동일하게 설정
    AttackerPosition.Z = DefenderTileCenter.Z;

    return AttackerPosition;
}
FVector UBTTask_MoveAttackerToDefenderTile::GetWorldPositionFromHex(FVector2D HexPosition, AUnitAIController* AIController) const
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


