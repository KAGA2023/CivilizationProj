// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Notify_Hit.generated.h"

/**
 * 공격 몽타주에서 중간 칼 휘두르는 타이밍에 호출되는 노티파이
 * 무조건 Hit 몽타주만 재생 (Death 없음). 연타 맞는 연출용.
 * 마지막 칼 휘두르는 타이밍에는 Notify_Combat 사용.
 */
UCLASS()
class CIVILIZATION_API UNotify_Hit : public UAnimNotify
{
	GENERATED_BODY()

public:
	UNotify_Hit();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
