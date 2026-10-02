// Fill out your copyright notice in the Description page of Project Settings.

#include "SuperCameraPawn.h"

ASuperCameraPawn::ASuperCameraPawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root Component (Scene Component)
	USceneComponent* RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	RootComponent = RootScene;

	// Spring Arm Component (카메라 줌/높이 조절용)
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 2000.0f; // 기본 높이
	SpringArm->SetRelativeRotation(FRotator(-60.0f, 0.0f, 0.0f)); // 60도 각도로 아래를 봄
	SpringArm->bDoCollisionTest = false; // 충돌 테스트 비활성화
	SpringArm->bEnableCameraLag = true; // 부드러운 카메라 이동
	SpringArm->CameraLagSpeed = 10.0f;
	SpringArm->bUseCameraLagSubstepping = true; // 카메라 위치 보간

	// Camera Component
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;

	// 줌 보간 설정
	TargetZoomDistance = 2000.0f;
	ZoomInterpSpeed = 8.0f; // 줌 보간 속도 (높을수록 빠름)

	// 기본 입력 비활성화 (Controller가 직접 제어)
	AutoPossessPlayer = EAutoReceiveInput::Disabled;
}

void ASuperCameraPawn::BeginPlay()
{
	Super::BeginPlay();
}

void ASuperCameraPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 줌 거리 부드럽게 보간
	if (SpringArm)
	{
		float CurrentLength = SpringArm->TargetArmLength;
		float NewLength = FMath::FInterpTo(CurrentLength, TargetZoomDistance, DeltaTime, ZoomInterpSpeed);
		SpringArm->TargetArmLength = NewLength;
	}
}

void ASuperCameraPawn::SetCameraHeight(float Height)
{
	if (SpringArm)
	{
		SpringArm->TargetArmLength = FMath::Clamp(Height, 500.0f, 5000.0f);
	}
}

void ASuperCameraPawn::SetCameraAngle(float Angle)
{
	if (SpringArm)
	{
		FRotator NewRotation = SpringArm->GetRelativeRotation();
		NewRotation.Pitch = FMath::Clamp(Angle, -80.0f, -30.0f);
		SpringArm->SetRelativeRotation(NewRotation);
	}
}

void ASuperCameraPawn::SetTargetZoomDistance(float Distance)
{
	TargetZoomDistance = FMath::Clamp(Distance, 500.0f, 5000.0f);
}

