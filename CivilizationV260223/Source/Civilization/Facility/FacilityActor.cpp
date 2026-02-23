// Fill out your copyright notice in the Description page of Project Settings.

#include "FacilityActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

AFacilityActor::AFacilityActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 씬 컴포넌트 생성
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// 시설 메시 컴포넌트 생성
	FacilityMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FacilityMesh"));
	FacilityMesh->SetupAttachment(RootSceneComponent);
	FacilityMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 시설은 콜리전 없음
	FacilityMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 10.0f)); // 타일 위에 약간 올림

	// 멤버 변수 초기화
	bIsPillaged = false;
	FacilityMeshAsset = nullptr;
	PillagedMeshAsset = nullptr;
}

void AFacilityActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 시각적 업데이트 (메시가 설정된 경우에만)
	UpdateVisual();
}

void AFacilityActor::SetFacilityMesh(UStaticMesh* InFacilityMesh, UStaticMesh* InPillagedMesh)
{
	FacilityMeshAsset = InFacilityMesh;
	PillagedMeshAsset = InPillagedMesh;
	UpdateVisual();
}

void AFacilityActor::SetPillaged(bool bInIsPillaged)
{
	bIsPillaged = bInIsPillaged;
	UpdateVisual();
}

void AFacilityActor::UpdateVisual()
{
	if (!FacilityMesh)
	{
		return;
	}

	// 약탈 상태에 따라 메시 선택
	UStaticMesh* MeshToUse = bIsPillaged && PillagedMeshAsset ? PillagedMeshAsset : FacilityMeshAsset;

	FacilityMesh->SetStaticMesh(MeshToUse);
}

void AFacilityActor::RepairFacility()
{
	SetPillaged(false);
}

