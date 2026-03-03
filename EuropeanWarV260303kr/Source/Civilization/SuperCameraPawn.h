// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "SuperCameraPawn.generated.h"

UCLASS()
class CIVILIZATION_API ASuperCameraPawn : public APawn
{
	GENERATED_BODY()

public:
	ASuperCameraPawn();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;

	// 카메라 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	float TargetZoomDistance; // 줌 보간용 변수

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomInterpSpeed; // 줌 보간 속도

	// 카메라 설정
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraHeight(float Height);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraAngle(float Angle);

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetTargetZoomDistance(float Distance);
};

