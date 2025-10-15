// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldTileActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"

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

	// 바다 메시 로드
	static ConstructorHelpers::FObjectFinder<UStaticMesh> OceanMeshAsset(TEXT("/Game/CartoonCuteNaturePack/Static_Meshes/Spots/SM_BAVEL_SPOT_WATER.SM_BAVEL_SPOT_WATER"));
	if (OceanMeshAsset.Succeeded())
	{
		OceanMesh = OceanMeshAsset.Object;
	}
	else
	{
		OceanMesh = nullptr;
	}

	// 온대 기후 메시 로드
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TemperateMeshAsset(TEXT("/Game/CartoonCuteNaturePack/Static_Meshes/Spots/SM_BAVEL_SPOT_DARK_GREEN.SM_BAVEL_SPOT_DARK_GREEN"));
	if (TemperateMeshAsset.Succeeded())
	{
		TemperateMesh = TemperateMeshAsset.Object;
	}
	else
	{
		TemperateMesh = nullptr;
	}

	// 사막 기후 메시 로드
	static ConstructorHelpers::FObjectFinder<UStaticMesh> DesertMeshAsset(TEXT("/Game/CartoonCuteNaturePack/Static_Meshes/Spots/SM_BAVEL_SPOT_SAND.SM_BAVEL_SPOT_SAND"));
	if (DesertMeshAsset.Succeeded())
	{
		DesertMesh = DesertMeshAsset.Object;
	}
	else
	{
		DesertMesh = nullptr;
	}

	// 툰드라 기후 메시 로드
	static ConstructorHelpers::FObjectFinder<UStaticMesh> TundraMeshAsset(TEXT("/Game/CartoonCuteNaturePack/Static_Meshes/Spots/SM_BAVEL_SPOT_ICE.SM_BAVEL_SPOT_ICE"));
	if (TundraMeshAsset.Succeeded())
	{
		TundraMesh = TundraMeshAsset.Object;
	}
	else
	{
		TundraMesh = nullptr;
	}
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
		
		// 지형 타입에 따라 Z축 스케일 조정
		FName LandTypeID = TileData->GetLandTypeID();
		FVector Scale = FVector(1.0f, 1.0f, 1.0f);
		
		if (LandTypeID == FName("Hills"))
		{
			Scale.Z = 2.0f; // 언덕은 Z축 스케일 2.0
		}
		else if (LandTypeID == FName("Mountains"))
		{
			Scale.Z = 3.0f; // 산은 Z축 스케일 3.0
		}
		
		SetActorScale3D(Scale);
		
		// 외형 업데이트
		UpdateVisual();
	}
}

void AWorldTileActor::UpdateVisual()
{
	if (!TileData)
	{
		return;
	}

	// 지형 타입에 따라 메시 변경
	switch (TileData->GetTerrainType())
	{
	case ETerrainType::Land:
		{
			// 기후대에 따라 메시 선택
			FName ClimateID = TileData->GetClimateTypeID();
			UStaticMesh* SelectedMesh = nullptr;

			if (ClimateID == FName("Temperate"))
			{
				SelectedMesh = TemperateMesh;
			}
			else if (ClimateID == FName("Desert"))
			{
				SelectedMesh = DesertMesh;
			}
			else if (ClimateID == FName("Tundra"))
			{
				SelectedMesh = TundraMesh;
			}
			else
			{
				// 기본값: Temperate
				SelectedMesh = TemperateMesh;
			}

			// 메시 적용 (메시 자체의 머티리얼 사용)
			if (SelectedMesh && TileMesh)
			{
				TileMesh->SetStaticMesh(SelectedMesh);
			}
		}
		break;
		
	case ETerrainType::Ocean:
		// 바다 타일 - 메시 설정 (메시 자체의 머티리얼 사용)
		if (OceanMesh && TileMesh)
		{
			TileMesh->SetStaticMesh(OceanMesh);
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
		// 타일 좌표 로그 출력
		if (TileData)
		{
			FVector2D HexPos = TileData->GetGridPosition();
			FVector WorldPos = TileData->GetWorldPosition();
			FString ClimateID = TileData->GetClimateTypeID().ToString();
			FString LandTypeID = TileData->GetLandTypeID().ToString();
			
			UE_LOG(LogTemp, Warning, TEXT("===== 타일 클릭 ====="));
			UE_LOG(LogTemp, Warning, TEXT("육각형 좌표: Q=%d, R=%d"), (int32)HexPos.X, (int32)HexPos.Y);
			UE_LOG(LogTemp, Warning, TEXT("월드 좌표: X=%.1f, Y=%.1f, Z=%.1f"), WorldPos.X, WorldPos.Y, WorldPos.Z);
			UE_LOG(LogTemp, Warning, TEXT("기후대: %s"), *ClimateID);
			UE_LOG(LogTemp, Warning, TEXT("지형: %s"), *LandTypeID);
			UE_LOG(LogTemp, Warning, TEXT("===================="));
		}
		
		// 타일 선택
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

