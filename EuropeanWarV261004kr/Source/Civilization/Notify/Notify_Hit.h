// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Notify_Hit.generated.h"

UCLASS()
class CIVILIZATION_API UNotify_Hit : public UAnimNotify
{
	GENERATED_BODY()

public:
	UNotify_Hit();

	// 연타 중간 타이밍에 Hit 몽타주만 재생합니다. 사망 처리는 하지 않습니다.
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
