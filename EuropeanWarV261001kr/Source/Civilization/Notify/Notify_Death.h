// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Notify_Death.generated.h"

/**
 * 사망 몽타주에서 사망 애니메이션이 끝나는 타이밍에 호출되는 노티파이
 * 방어자 유닛을 destroy 처리 (공격자는 반격으로 죽을 수 없음)
 */
UCLASS()
class CIVILIZATION_API UNotify_Death : public UAnimNotify
{
    GENERATED_BODY()

public:
    UNotify_Death();

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

