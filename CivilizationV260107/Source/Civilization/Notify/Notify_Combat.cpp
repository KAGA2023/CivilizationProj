// Fill out your copyright notice in the Description page of Project Settings.

#include "Notify_Combat.h"
#include "../Unit/UnitCharacterBase.h"
#include "../Unit/UnitVisualizationComponent.h"

UNotify_Combat::UNotify_Combat()
{
}

void UNotify_Combat::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::Notify(MeshComp, Animation, EventReference);

    if (!MeshComp)
    {
        return;
    }

    // MeshComp의 소유자(공격자) 가져오기
    AUnitCharacterBase* Attacker = Cast<AUnitCharacterBase>(MeshComp->GetOwner());
    if (!Attacker)
    {
        return;
    }

    // 공격자의 UnitVisualizationComponent 가져오기
    UUnitVisualizationComponent* VisComp = Attacker->GetUnitVisualizationComponent();
    if (!VisComp)
    {
        return;
    }

    // 전투 중이고 공격 상태일 때만 실행
    if (VisComp->IsInCombat())
    {
        // 공격 노티파이 콜백 호출
        VisComp->OnCombatNotify_Hit();
    }
}

