// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_MoveAttackerToDefenderTile.generated.h"

UCLASS()
class CIVILIZATION_API UBTTask_MoveAttackerToDefenderTile : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_MoveAttackerToDefenderTile();

protected:
    // Task 실행
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    
    // Task 틱 (도착 확인)
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    // Task 종료
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
    // 도착 판단 반경
    UPROPERTY(EditAnywhere, Category = "Movement", meta = (ClampMin = "0"))
    float AcceptanceRadius = 20.0f;

    // 마지막으로 점프한 목표 타일 좌표 (각 타일 이동마다 한 번만 점프하기 위함)
    FVector2D LastJumpedTargetHex = FVector2D::ZeroVector;

    // 공격자의 시작 타일 좌표 (점프 조건 체크를 위해 저장)
    FVector2D AttackerStartHex = FVector2D::ZeroVector;

    // 방어자 타일의 공격자 배치 위치 계산 (방어자와 겹치지 않는 위치)
    FVector CalculateAttackerPositionOnDefenderTile(class AUnitAIController* AIController, class AUnitCharacterBase* Defender) const;

    // 헥스 좌표를 월드 좌표로 변환 (같은 높이만 고려)
    FVector GetWorldPositionFromHex(FVector2D HexPosition, class AUnitAIController* AIController) const;
};

