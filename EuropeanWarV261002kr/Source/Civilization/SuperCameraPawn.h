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

	// ========== 카메라 컴포넌트 ==========

	// 줌·높이를 담당하는 스프링 암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	// 실제 렌더 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	// 스프링 암이 보간할 목표 줌 거리
	UPROPERTY(BlueprintReadWrite, Category = "Camera")
	float TargetZoomDistance;

	// 줌 보간 속도. 클수록 목표 거리에 빨리 붙습니다.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomInterpSpeed;

	// ========== 카메라 설정 ==========

	// 스프링 암 길이를 바로 바꿉니다. 500~5000으로 클램프합니다.
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraHeight(float Height);

	// 카메라 피치를 바꿉니다. -80~-30으로 클램프합니다.
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetCameraAngle(float Angle);

	// 목표 줌 거리를 설정합니다. Tick에서 부드럽게 보간합니다.
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetTargetZoomDistance(float Distance);
};
