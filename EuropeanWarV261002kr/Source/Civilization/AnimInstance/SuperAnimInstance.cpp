// Fill out your copyright notice in the Description page of Project Settings.

#include "SuperAnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"

USuperAnimInstance::USuperAnimInstance()
{
	OwningPlayer = nullptr;
	CharacterMovement = nullptr;

	Velocity = FVector::ZeroVector;
	GroundSpeed = 0.0f;
	bShouldMove = false;
	bIsFalling = false;
}

void USuperAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	AActor* Owner = GetOwningActor();

	if (Owner)
	{
		OwningPlayer = Cast<ACharacter>(Owner);

		if (OwningPlayer)
		{
			CharacterMovement = OwningPlayer->GetCharacterMovement();
		}
	}
}

void USuperAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (CharacterMovement)
	{
		Velocity = CharacterMovement->Velocity;

		GroundSpeed = FMath::Lerp(GroundSpeed, Velocity.Size2D(), DeltaSeconds * 10.0f);

		bShouldMove = GroundSpeed > 3.0f;

		bIsFalling = CharacterMovement->IsFalling();
	}
	else
	{
		Velocity = FVector::ZeroVector;
		GroundSpeed = 0.0f;
		bShouldMove = false;
		bIsFalling = false;
	}
}
