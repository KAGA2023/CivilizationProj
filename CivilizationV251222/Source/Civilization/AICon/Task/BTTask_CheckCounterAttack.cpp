// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_CheckCounterAttack.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../UnitAIController.h"
#include "../../Combat/UnitCombatStruct.h"

UBTTask_CheckCounterAttack::UBTTask_CheckCounterAttack()
{
    NodeName = TEXT("Check Counter Attack");
    bCreateNodeInstance = false;
    
    // Blackboard 키 기본값 설정 (선택적)
    BBKey_CanCounterAttack.SelectedKeyName = TEXT("CanCounterAttack");
}

EBTNodeResult::Type UBTTask_CheckCounterAttack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

    // 반격 가능 여부 확인
    // 1. 방어자가 반격 데미지를 줬는지 (DefenderDamageDealt > 0)
    // 2. 둘 다 생존했는지
    bool bCanCounterAttack = (CombatResult.DefenderDamageDealt > 0) && CombatResult.bAttackerAlive && CombatResult.bDefenderAlive;

    // Blackboard에 반격 가능 여부 저장
    UBlackboardComponent* Blackboard = OwnerComp.GetBlackboardComponent();
    if (Blackboard && BBKey_CanCounterAttack.SelectedKeyName != NAME_None)
    {
        Blackboard->SetValueAsBool(BBKey_CanCounterAttack.SelectedKeyName, bCanCounterAttack);
    }

    // 반격 가능하면 성공, 불가능하면 실패
    // (비헤이비어트리에서 Decorator로 조건부 분기 가능)
    return bCanCounterAttack ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}

