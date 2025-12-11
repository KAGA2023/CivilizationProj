// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CompleteCombatVisualization.generated.h"

UCLASS()
class CIVILIZATION_API UBTTask_CompleteCombatVisualization : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_CompleteCombatVisualization();

protected:
    // Task 실행
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
};

