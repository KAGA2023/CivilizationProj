// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "InputMappingContext.h"
#include "SuperGameController.generated.h"

/**
 * 문명5 스타일의 게임 컨트롤러
 * 카메라 컨트롤과 기본 입력 처리를 담당합니다.
 */
UCLASS()
class CIVILIZATION_API ASuperGameController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// Enhanced Input 설정
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputMappingContext* IMC_InGame; // 인게임 입력 매핑 컨텍스트

	// 마우스 입력 액션들
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_LMB; // 좌클릭

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_RMB; // 우클릭

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_Wheel; // 마우스 휠

	// 입력 처리 함수들
	void OnLMB_Pressed(); // 좌클릭 시작
	void OnLMB_Released(); // 좌클릭 종료
	void OnRMB_Pressed(); // 우클릭 시작
	void OnRMB_Released(); // 우클릭 종료
	void OnWheel_Triggered(const FInputActionInstance& Instance); // 마우스 휠

	// 카메라 컨트롤 변수들
	bool bIsPanning; // 카메라 팬 상태
	FVector2D LastMousePosition; // 마지막 마우스 위치
	FVector2D CurrentMousePosition; // 현재 마우스 위치

	// 카메라 설정
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float PanSpeed; // 카메라 팬 속도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ZoomSpeed; // 카메라 줌 속도

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MinZoomDistance; // 최소 줌 거리

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxZoomDistance; // 최대 줌 거리

	// 카메라 컨트롤 함수들
	void UpdateCameraPan(); // 카메라 팬 업데이트
	void UpdateCameraZoom(float ZoomDelta); // 카메라 줌 업데이트
	FVector2D GetMousePosition() const; // 마우스 위치 가져오기
	void SetCameraLocation(FVector NewLocation); // 카메라 위치 설정
	void SetCameraRotation(FRotator NewRotation); // 카메라 회전 설정

public:
	ASuperGameController();

	virtual void Tick(float DeltaTime) override;

	// 카메라 관련 함수들
	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetPanSpeed(float NewSpeed) { PanSpeed = NewSpeed; } // 팬 속도 설정

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetZoomSpeed(float NewSpeed) { ZoomSpeed = NewSpeed; } // 줌 속도 설정

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void SetZoomLimits(float MinDistance, float MaxDistance); // 줌 범위 설정

	UFUNCTION(BlueprintCallable, Category = "Camera")
	bool IsCameraPanning() const { return bIsPanning; } // 카메라 팬 상태 확인
};
