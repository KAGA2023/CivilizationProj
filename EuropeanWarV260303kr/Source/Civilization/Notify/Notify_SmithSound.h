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

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
