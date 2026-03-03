// Fill out your copyright notice in the Description page of Project Settings.

#include "Notify_Hit.h"
#include "../Unit/UnitCharacterBase.h"
#include "../Unit/UnitVisualizationComponent.h"

UNotify_Hit::UNotify_Hit()
{
}

void UNotify_Hit::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	// MeshComp의 소유자(공격자 또는 반격 중 방어자) 가져오기
	AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(MeshComp->GetOwner());
	if (!Unit)
	{
		return;
	}

	UUnitVisualizationComponent* VisComp = Unit->GetUnitVisualizationComponent();
	if (!VisComp)
	{
		return;
	}

	if (VisComp->IsInCombat())
	{
		// Hit만 재생 (Death 없음)
		VisComp->OnCombatNotify_HitOnly();
	}
}
