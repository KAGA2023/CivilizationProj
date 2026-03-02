// Fill out your copyright notice in the Description page of Project Settings.

#include "Notify_Death.h"
#include "../Unit/UnitCharacterBase.h"
#include "../Unit/UnitVisualizationComponent.h"

UNotify_Death::UNotify_Death()
{
}

void UNotify_Death::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp)
    {
        return;
    }

    // MeshComp의 소유자(방어자) 가져오기
    // 주의: 공격자는 반격으로 죽을 수 없으므로 이 노티파이는 방어자에게만 적용됨
    AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(MeshComp->GetOwner());
    if (!Unit)
    {
        return;
    }

    // Unit의 UnitVisualizationComponent 가져오기
    UUnitVisualizationComponent* VisComp = Unit->GetUnitVisualizationComponent();
    if (!VisComp)
    {
        return;
    }

    // 전투 중일 때만 실행
    if (VisComp->IsInCombat())
    {
        // 사망 노티파이 콜백 호출
        VisComp->OnCombatNotify_Death();
    }
}

