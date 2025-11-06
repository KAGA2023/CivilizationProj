// Fill out your copyright notice in the Description page of Project Settings.

#include "SuperGameController.h"
#include "Camera/CameraComponent.h"
#include "SuperCameraPawn.h"

ASuperGameController::ASuperGameController()
{
	// Enhanced Input 초기화
	IMC_InGame = nullptr;
	IA_LMB = nullptr;
	IA_RMB = nullptr;
	IA_Wheel = nullptr;

	// 카메라 컨트롤 초기화
	bIsPanning = false;
	LastMousePosition = FVector2D::ZeroVector;
	CurrentMousePosition = FVector2D::ZeroVector;

	// 카메라 설정 기본값
	PanSpeed = 3.0f;
	ZoomSpeed = 200.0f; // 줌 속도 낮춰서 부드럽게
	MinZoomDistance = 500.0f;
	MaxZoomDistance = 5000.0f;

	PrimaryActorTick.bCanEverTick = true;
}

void ASuperGameController::BeginPlay()
{
	Super::BeginPlay();

	// 로컬 플레이어인지 확인
	if (!IsLocalController()) return;

	// Enhanced Input 설정
	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
		{
			Subsystem->ClearAllMappings();
			if (IMC_InGame)
			{
				Subsystem->AddMappingContext(IMC_InGame, 0);
			}
		}
	}

	// 마우스 커서 표시
	bShowMouseCursor = true;
	
	// 클릭 이벤트 활성화 (타일 클릭 감지용)
	bEnableClickEvents = true;
	bEnableMouseOverEvents = true;

	// 입력 모드 설정 (게임 + UI)
	FInputModeGameAndUI InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::LockAlways);
	InputMode.SetHideCursorDuringCapture(false);
	SetInputMode(InputMode);
}

void ASuperGameController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// Enhanced Input Component로 캐스팅
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
	{
		// 마우스 입력 바인딩 (null 체크 추가)
		if (IA_LMB)
		{
			EnhancedInputComponent->BindAction(IA_LMB, ETriggerEvent::Started, this, &ASuperGameController::OnLMB_Pressed);
			EnhancedInputComponent->BindAction(IA_LMB, ETriggerEvent::Completed, this, &ASuperGameController::OnLMB_Released);
		}
		if (IA_RMB)
		{
			EnhancedInputComponent->BindAction(IA_RMB, ETriggerEvent::Started, this, &ASuperGameController::OnRMB_Pressed);
			EnhancedInputComponent->BindAction(IA_RMB, ETriggerEvent::Completed, this, &ASuperGameController::OnRMB_Released);
		}
		if (IA_Wheel)
		{
			EnhancedInputComponent->BindAction(IA_Wheel, ETriggerEvent::Triggered, this, &ASuperGameController::OnWheel_Triggered);
		}
	}
}

void ASuperGameController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 카메라 팬 업데이트
	if (bIsPanning)
	{
		UpdateCameraPan();
	}
}

void ASuperGameController::OnLMB_Pressed()
{
	// 좌클릭 시작 - 타일 클릭 전용 (WorldTileActor에서 처리)
}

void ASuperGameController::OnLMB_Released()
{
	// 좌클릭 종료
}

void ASuperGameController::OnRMB_Pressed()
{
	// 우클릭 시작 - 카메라 팬 시작
	bIsPanning = true;
	LastMousePosition = GetMousePosition();
}

void ASuperGameController::OnRMB_Released()
{
	// 우클릭 종료 - 카메라 팬 종료
	bIsPanning = false;
}

void ASuperGameController::OnWheel_Triggered(const FInputActionInstance& Instance)
{
	// 마우스 휠 - 카메라 줌
	float ZoomDelta = Instance.GetValue().Get<float>();
	UpdateCameraZoom(ZoomDelta);
}

void ASuperGameController::UpdateCameraPan()
{
	// 현재 마우스 위치 가져오기
	CurrentMousePosition = GetMousePosition();

	// 마우스 이동량 계산
	FVector2D MouseDelta = CurrentMousePosition - LastMousePosition;

	// 카메라 팬 계산
	if (APawn* ControlledPawn = GetPawn())
	{
		// 카메라의 현재 위치와 회전 가져오기
		FVector CurrentLocation = ControlledPawn->GetActorLocation();
		FRotator CurrentRotation = ControlledPawn->GetActorRotation();

		// 마우스 이동량을 월드 좌표로 변환
		FVector RightVector = CurrentRotation.RotateVector(FVector::RightVector);
		FVector ForwardVector = CurrentRotation.RotateVector(FVector::ForwardVector);

		// 팬 방향 계산 (마우스 X축 = 카메라 오른쪽, 마우스 Y축 = 카메라 앞쪽)
		FVector PanDirection = (RightVector * -MouseDelta.X + ForwardVector * MouseDelta.Y);
		PanDirection.Z = 0.0f; // Z축은 고정 (수평 이동만)

		// 새로운 카메라 위치 계산
		FVector NewLocation = CurrentLocation + PanDirection * PanSpeed;

		// 카메라 위치 설정
		SetCameraLocation(NewLocation);
	}

	// 마지막 마우스 위치 업데이트
	LastMousePosition = CurrentMousePosition;
}

void ASuperGameController::UpdateCameraZoom(float ZoomDelta)
{
	if (ASuperCameraPawn* CameraPawn = Cast<ASuperCameraPawn>(GetPawn()))
	{
		// 목표 줌 거리 설정 (Pawn의 Tick에서 보간됨)
		float CurrentTarget = CameraPawn->TargetZoomDistance;
		float NewTarget = CurrentTarget - (ZoomDelta * ZoomSpeed);
		
		// 줌 범위 제한
		NewTarget = FMath::Clamp(NewTarget, MinZoomDistance, MaxZoomDistance);
		
		CameraPawn->SetTargetZoomDistance(NewTarget);
	}
}

FVector2D ASuperGameController::GetMousePosition() const
{
	float MouseX, MouseY;
	if (APlayerController::GetMousePosition(MouseX, MouseY))
	{
		return FVector2D(MouseX, MouseY);
	}
	return FVector2D::ZeroVector;
}

void ASuperGameController::SetCameraLocation(FVector NewLocation)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->SetActorLocation(NewLocation);
	}
}

void ASuperGameController::SetCameraRotation(FRotator NewRotation)
{
	if (APawn* ControlledPawn = GetPawn())
	{
		ControlledPawn->SetActorRotation(NewRotation);
	}
}

void ASuperGameController::SetZoomLimits(float MinDistance, float MaxDistance)
{
	MinZoomDistance = FMath::Max(MinDistance, 100.0f);
	MaxZoomDistance = FMath::Max(MaxDistance, MinZoomDistance + 100.0f);
}
