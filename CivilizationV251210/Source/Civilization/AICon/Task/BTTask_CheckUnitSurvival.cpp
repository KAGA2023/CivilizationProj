// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_CheckUnitSurvival.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../UnitAIController.h"
#include "../../Combat/UnitCombatStruct.h"

UBTTask_CheckUnitSurvival::UBTTask_CheckUnitSurvival()
{
    NodeName = TEXT("Check Unit Survival");
    bCreateNodeInstance = false;
    
    // Blackboard 키 기본값 설정 (선택적)
    BBKey_AttackerAlive.SelectedKeyName = TEXT("AttackerAlive");
    BBKey_DefenderAlive.SelectedKeyName = TEXT("DefenderAlive");
}

EBTNodeResult::Type UBTTask_CheckUnitSurvival::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // 전투 결과 가져오기
    FCombatResult CombatResult = UnitAIController->GetCurrentCombatResult();

    // Blackboard에 생존 여부 저장
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (Blackboard)
    {
        // Blackboard 키가 설정되어 있으면 값 저장
        if (BBKey_AttackerAlive.SelectedKeyName != NAME_None)
        {
            Blackboard->SetValueAsBool(BBKey_AttackerAlive.SelectedKeyName, CombatResult.bAttackerAlive);
        }

        if (BBKey_DefenderAlive.SelectedKeyName != NAME_None)
        {
            Blackboard->SetValueAsBool(BBKey_DefenderAlive.SelectedKeyName, CombatResult.bDefenderAlive);
        }
    }

    // 둘 다 생존했으면 성공, 하나라도 죽었으면 실패
    // (비헤이비어트리에서 Decorator로 조건부 분기 가능)
    if (CombatResult.bAttackerAlive && CombatResult.bDefenderAlive)
    {
        return EBTNodeResult::Succeeded;
    }
    else
    {
        return EBTNodeResult::Failed;
    }
}

