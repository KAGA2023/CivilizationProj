// Fill out your copyright notice in the Description page of Project Settings.

#include "Notify_SmithSound.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "../Unit/UnitCharacterBase.h"

UNotify_SmithSound::UNotify_SmithSound()
{
}

void UNotify_SmithSound::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (!MeshComp)
	{
		return;
	}

	// 플레이어 0(본인)의 건설자만 Smith 사운드 재생
	AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(MeshComp->GetOwner());
	if (!Unit || Unit->GetPlayerIndex() != 0)
	{
		return;
	}

	USoundBase* SmithSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Civilization/Sound/UnitAttack/Smith.Smith"));
	if (!SmithSound)
	{
		return;
	}

	UWorld* World = MeshComp->GetWorld();
	if (World)
	{
		UGameplayStatics::PlaySoundAtLocation(World, SmithSound, MeshComp->GetComponentLocation());
	}
}
