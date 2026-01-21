// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Notify_Combat.generated.h"

/**
 * 공격 몽타주에서 무기 휘두르는 타이밍에 호출되는 노티파이
 * 방어자의 피격/사망 몽타주를 즉시 실행
 */
UCLASS()
class CIVILIZATION_API UNotify_Combat : public UAnimNotify
{
    GENERATED_BODY()

public:
    UNotify_Combat();

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};

