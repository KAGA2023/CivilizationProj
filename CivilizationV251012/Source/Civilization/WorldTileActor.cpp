// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldTileActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"

AWorldTileActor::AWorldTileActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 컴포넌트로 StaticMesh 생성
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	RootComponent = TileMesh;

	// 클릭 가능하도록 설정
	TileMesh->SetGenerateOverlapEvents(true);
	TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TileMesh->SetCollisionResponseToAllChannels(ECR_Block);
	TileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 클릭 감지

	// 클릭 이벤트 바인딩
	TileMesh->OnClicked.AddDynamic(this, &AWorldTileActor::OnTileClicked);

	// 기본값 초기화
	TileData = nullptr;
	bIsSelected = false;
	BaseMaterial = nullptr;
	DynamicMaterial = nullptr;
	HighlightColor = FLinearColor::Yellow;
}

void AWorldTileActor::BeginPlay()
{
	Super::BeginPlay();
	
	// 다이나믹 머티리얼 인스턴스 생성
	if (BaseMaterial && TileMesh)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, this);
		TileMesh->SetMaterial(0, DynamicMaterial);
	}
}

void AWorldTileActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWorldTileActor::SetTileData(UWorldTile* NewTileData)
{
	TileData = NewTileData;
	
	if (TileData)
	{
		// 타일 데이터에 따라 위치 설정
		FVector WorldPos = TileData->GetWorldPosition();
		SetActorLocation(WorldPos);
		
		// 외형 업데이트
		UpdateVisual();
	}
}

void AWorldTileActor::UpdateVisual()
{
	if (!TileData || !DynamicMaterial)
	{
		return;
	}

	// TODO: 타일 타입에 따라 머티리얼/색상 변경
	// 예시: 지형 타입에 따라 색상 변경
	switch (TileData->GetTerrainType())
	{
	case ETerrainType::Land:
		// 땅 타일 - 초록색 계열
		if (DynamicMaterial)
		{
			DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), FLinearColor::Green);
		}
		break;
	case ETerrainType::Ocean:
		// 바다 타일 - 파란색 계열
		if (DynamicMaterial)
		{
			DynamicMaterial->SetVectorParameterValue(FName("BaseColor"), FLinearColor::Blue);
		}
		break;
	default:
		break;
	}

	// 선택 상태 반영
	if (bIsSelected && DynamicMaterial)
	{
		DynamicMaterial->SetVectorParameterValue(FName("HighlightColor"), HighlightColor);
		DynamicMaterial->SetScalarParameterValue(FName("HighlightIntensity"), 1.0f);
	}
	else if (DynamicMaterial)
	{
		DynamicMaterial->SetScalarParameterValue(FName("HighlightIntensity"), 0.0f);
	}
}

void AWorldTileActor::OnTileClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	// 좌클릭만 처리
	if (ButtonPressed == EKeys::LeftMouseButton)
	{
		UE_LOG(LogTemp, Log, TEXT("[WorldTileActor] Tile Clicked: %s"), *GetName());
		
		// TODO: 타일 선택 로직 (WorldComponent에 알림)
		SetSelected(!bIsSelected);
	}
}

void AWorldTileActor::SetSelected(bool bSelected)
{
	bIsSelected = bSelected;
	
	// 타일 데이터에도 반영
	if (TileData)
	{
		TileData->SetSelected(bSelected);
	}
	
	// 외형 업데이트
	UpdateVisual();
}

