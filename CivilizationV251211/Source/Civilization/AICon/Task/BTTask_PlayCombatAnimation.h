// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PlayCombatAnimation.generated.h"

UCLASS()
class CIVILIZATION_API UBTTask_PlayCombatAnimation : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_PlayCombatAnimation();

protected:
    // Task 실행
    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    
    // Task 틱 (애니메이션 완료 확인)
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

    // Task 종료
    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

private:
    // 애니메이션 Montage 재생
    bool PlayAnimationMontage(class AUnitCharacterBase* Unit, class UAnimMontage* Montage) const;
    
    // 애니메이션 재생 중인지 확인
    bool IsAnimationPlaying(class AUnitCharacterBase* Unit) const;
    
    // FUnitData에서 Montage 가져오기
    class UAnimMontage* GetCombatMontage(class AUnitCharacterBase* Unit, bool bIsAttacker) const;
};

