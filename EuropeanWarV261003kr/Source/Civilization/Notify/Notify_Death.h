// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Notify_Death.generated.h"

UCLASS()
class CIVILIZATION_API UNotify_Death : public UAnimNotify
{
    GENERATED_BODY()

public:
    UNotify_Death();

    // 사망 몽타주가 끝나면 방어자 유닛을 제거합니다. 공격자는 반격으로 죽지 않습니다.
    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
