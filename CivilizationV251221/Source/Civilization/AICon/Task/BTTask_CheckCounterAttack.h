// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_CheckCounterAttack.generated.h"

UCLASS()
class CIVILIZATION_API UBTTask_CheckCounterAttack : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_CheckCounterAttack();

protected:
    // Task 실행
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

private:
    // Blackboard 키 선택자 (반격 가능 여부 저장용)
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector BBKey_CanCounterAttack;
};

