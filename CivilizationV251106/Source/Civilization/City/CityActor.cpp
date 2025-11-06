// Fill out your copyright notice in the Description page of Project Settings.

#include "CityActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"

ACityActor::ACityActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// 도시 메시
	CityMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CityMesh"));
	CityMesh->SetupAttachment(RootSceneComponent);
	CityMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CityMesh->SetGenerateOverlapEvents(false);

	// 기본 메시 로드 및 설정
	{
		static ConstructorHelpers::FObjectFinder<UStaticMesh> CityMeshAsset(TEXT("/Game/StarterContent/Props/SM_Chair.SM_Chair"));
		if (CityMeshAsset.Succeeded())
		{
			CityMesh->SetStaticMesh(CityMeshAsset.Object);
		}
		// Z축(야우) 180도 회전
		CityMesh->SetRelativeRotation(FRotator(0.0f, 180.0f, 0.0f));
	}

	// 도시 로직 컴포넌트
	CityComponent = CreateDefaultSubobject<UCityComponent>(TEXT("CityComponent"));

	// 기본값
	bIsSelected = false;
}

void ACityActor::BeginPlay()
{
	Super::BeginPlay();
}

void ACityActor::InitializeCity(const FCityData& InCityData)
{
	if (CityComponent)
	{
		CityComponent->InitFromCityData(InCityData);
	}

	UpdateVisual();
}

void ACityActor::UpdateVisual()
{
	// 외형 갱신은 외부 시스템에서 처리. 현재 함수는 자리만 유지.
}

void ACityActor::SetSelected(bool bSelectedIn)
{
	bIsSelected = bSelectedIn;
	UpdateVisual();
}


