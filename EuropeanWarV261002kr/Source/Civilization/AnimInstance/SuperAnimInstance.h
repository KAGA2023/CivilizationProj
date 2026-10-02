// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "SuperAnimInstance.generated.h"

/**
 *
 */
UCLASS()
class CIVILIZATION_API USuperAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	// ========== 소유 캐릭터 ==========

	// 이 애님 인스턴스를 쓰는 캐릭터
	UPROPERTY(BlueprintReadOnly)
	ACharacter* OwningPlayer;

	// 소유 캐릭터의 이동 컴포넌트
	UPROPERTY(BlueprintReadOnly)
	UCharacterMovementComponent* CharacterMovement;

	// ========== 이동 상태 ==========

	// 현재 속도 벡터
	UPROPERTY(BlueprintReadOnly)
	FVector Velocity;

	// 지면 기준 이동 속력
	UPROPERTY(BlueprintReadOnly)
	float GroundSpeed;

	// 이동 애님으로 전환할지 여부
	UPROPERTY(BlueprintReadOnly)
	bool bShouldMove;

	// 낙하 중인지 여부
	UPROPERTY(BlueprintReadOnly)
	bool bIsFalling;

public:
	USuperAnimInstance();

protected:
	// 소유 캐릭터와 이동 컴포넌트를 연결합니다.
	virtual void NativeInitializeAnimation() override;

	// 속도·낙하 여부를 매 프레임 갱신합니다.
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
