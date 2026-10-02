// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Notify_SmithSound.generated.h"

UCLASS()
class CIVILIZATION_API UNotify_SmithSound : public UAnimNotify
{
	GENERATED_BODY()

public:
	UNotify_SmithSound();

	// 로컬 슬롯 유닛일 때만 타격 사운드를 재생합니다.
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
