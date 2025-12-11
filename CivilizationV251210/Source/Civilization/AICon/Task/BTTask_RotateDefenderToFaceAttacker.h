// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RotateDefenderToFaceAttacker.generated.h"

UCLASS()
class CIVILIZATION_API UBTTask_RotateDefenderToFaceAttacker : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_RotateDefenderToFaceAttacker();

protected:
    // Task 실행
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    
    // Task 틱 (회전 완료 확인)
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    // Task 종료
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
    // 회전 속도 (도/초)
    UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0"))
    float RotationSpeed = 180.0f;

    // 회전 완료 판단 각도 (도)
    UPROPERTY(EditAnywhere, Category = "Rotation", meta = (ClampMin = "0"))
    float RotationTolerance = 5.0f;
};

