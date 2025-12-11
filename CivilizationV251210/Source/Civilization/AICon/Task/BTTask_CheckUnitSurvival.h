// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CheckUnitSurvival.generated.h"

UCLASS()
class CIVILIZATION_API UBTTask_CheckUnitSurvival : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_CheckUnitSurvival();

protected:
    // Task 실행
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
    // Blackboard 키 선택자 (생존 여부 저장용)
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector BBKey_AttackerAlive;

    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector BBKey_DefenderAlive;
};

