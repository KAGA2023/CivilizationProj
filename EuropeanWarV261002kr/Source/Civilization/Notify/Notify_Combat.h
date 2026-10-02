// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Notify_Combat.generated.h"

UCLASS()
class CIVILIZATION_API UNotify_Combat : public UAnimNotify
{
    GENERATED_BODY()

public:
    UNotify_Combat();

    // 마지막 휘두르기 타이밍에 방어자의 피격/사망 몽타주를 실행합니다.
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
