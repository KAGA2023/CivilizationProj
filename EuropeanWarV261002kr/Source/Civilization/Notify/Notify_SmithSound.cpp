// Fill out your copyright notice in the Description page of Project Settings.

#include "Notify_SmithSound.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "../Unit/UnitCharacterBase.h"
#include "../SuperGameInstance.h"

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

	AUnitCharacterBase* Unit = Cast<AUnitCharacterBase>(MeshComp->GetOwner());
	if (!Unit)
	{
		return;
	}

	int32 LocalPlayerIndex = 0;
	if (UWorld* OwnerWorld = MeshComp->GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(OwnerWorld->GetGameInstance()))
		{
			LocalPlayerIndex = SuperGameInst->GetLocalPlayerIndex();
		}
	}
	if (Unit->GetPlayerIndex() != LocalPlayerIndex)
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
