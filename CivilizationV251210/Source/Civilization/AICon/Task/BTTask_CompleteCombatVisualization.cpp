// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_CompleteCombatVisualization.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "../UnitAIController.h"

UBTTask_CompleteCombatVisualization::UBTTask_CompleteCombatVisualization()
{
    NodeName = TEXT("Complete Combat Visualization");
    bCreateNodeInstance = false;
}

EBTNodeResult::Type UBTTask_CompleteCombatVisualization::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // 전투 시각화 완료 처리
    UnitAIController->CompleteCombatVisualization();

    return EBTNodeResult::Succeeded;
}

