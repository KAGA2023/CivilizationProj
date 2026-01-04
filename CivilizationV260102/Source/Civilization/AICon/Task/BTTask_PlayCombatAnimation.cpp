// Fill out your copyright notice in the Description page of Project Settings.

#include "BTTask_PlayCombatAnimation.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "../UnitAIController.h"
#include "../../Unit/UnitCharacterBase.h"
#include "../../Status/UnitDataStruct.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"
#include "Components/SkeletalMeshComponent.h"

UBTTask_PlayCombatAnimation::UBTTask_PlayCombatAnimation()
{
    NodeName = TEXT("Play Combat Animation");
    
    // Task가 틱을 사용하도록 설정
    bCreateNodeInstance = false;
    bNotifyTick = true; // TickTask를 사용하기 위해 필수!
}

EBTNodeResult::Type UBTTask_PlayCombatAnimation::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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
    AUnitCharacterBase* Attacker = UnitAIController->GetCombatAttacker();
    AUnitCharacterBase* Defender = UnitAIController->GetCombatDefender();
    
    if (!Attacker || !Defender)
    {
        return EBTNodeResult::Failed;
    }

    // 현재 Pawn이 공격자인지 방어자인지 확인
    AUnitCharacterBase* CurrentUnit = Cast<AUnitCharacterBase>(Pawn);
    if (!CurrentUnit)
    {
        return EBTNodeResult::Failed;
    }

    // 애니메이션 Montage 가져오기 (블루프린트에서 설정된 Montage 사용)
    // 현재는 Character의 AnimInstance에서 Montage를 가져오는 방식
    // 실제 구현에서는 블루프린트에서 Montage를 설정하거나 데이터 테이블에서 가져올 수 있음
    
    // 공격자/방어자 구분
    bool bIsAttacker = (CurrentUnit == Attacker);
    
    // FUnitData에서 적절한 Montage 가져오기
    UAnimMontage* MontageToPlay = GetCombatMontage(CurrentUnit, bIsAttacker);
    
    if (!MontageToPlay)
    {
        // Montage가 없으면 실패
        return EBTNodeResult::Failed;
    }
    
    // 애니메이션 Montage 재생
    if (!PlayAnimationMontage(CurrentUnit, MontageToPlay))
    {
        // 재생 실패
        return EBTNodeResult::Failed;
    }

    // 애니메이션 재생 중이므로 InProgress 반환 (TickTask에서 완료 확인)
    return EBTNodeResult::InProgress;
}

void UBTTask_PlayCombatAnimation::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
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

    AUnitCharacterBase* CurrentUnit = Cast<AUnitCharacterBase>(Pawn);
    if (!CurrentUnit)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 애니메이션 재생 완료 확인
    if (!IsAnimationPlaying(CurrentUnit))
    {
        // 애니메이션 재생 완료
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        return;
    }

    // 아직 재생 중이면 계속 대기
}

void UBTTask_PlayCombatAnimation::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

bool UBTTask_PlayCombatAnimation::PlayAnimationMontage(AUnitCharacterBase* Unit, UAnimMontage* Montage) const
{
    if (!Unit || !Montage)
    {
        return false;
    }

    if (USkeletalMeshComponent* Mesh = Unit->GetMesh())
    {
        if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
        {
            float PlayRate = 1.0f;
            float StartingPosition = 0.0f;
            FName StartingSection = NAME_None;
            
            float MontageLength = AnimInstance->Montage_Play(Montage, PlayRate, EMontagePlayReturnType::MontageLength, StartingPosition);
            return MontageLength > 0.0f;
        }
    }

    return false;
}

bool UBTTask_PlayCombatAnimation::IsAnimationPlaying(AUnitCharacterBase* Unit) const
{
    if (!Unit)
    {
        return false;
    }

    if (USkeletalMeshComponent* Mesh = Unit->GetMesh())
    {
        if (UAnimInstance* AnimInstance = Mesh->GetAnimInstance())
        {
            // Montage가 재생 중인지 확인
            return AnimInstance->Montage_IsPlaying(nullptr);
        }
    }

    return false;
}

UAnimMontage* UBTTask_PlayCombatAnimation::GetCombatMontage(AUnitCharacterBase* Unit, bool bIsAttacker) const
{
    if (!Unit)
    {
        return nullptr;
    }

    // UnitCharacterBase의 GetUnitData()를 통해 FUnitData 가져오기
    const FUnitData* UnitData = Unit->GetUnitData();
    if (!UnitData)
    {
        return nullptr;
    }

    // 공격자/방어자에 따라 적절한 Montage 반환
    if (bIsAttacker)
    {
        return UnitData->AttackMontage;
    }
    else
    {
        return UnitData->HitMontage;
    }
}

