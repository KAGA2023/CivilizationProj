// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldTileActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "WorldComponent.h"
#include "SuperGameInstance.h"

AWorldTileActor::AWorldTileActor()
{
	PrimaryActorTick.bCanEverTick = false;

	// 루트 씬 컴포넌트 생성
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	// 타일 메시 생성 (루트 씬 컴포넌트에 부착)
	TileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TileMesh"));
	TileMesh->SetupAttachment(RootSceneComponent);

	// 클릭 가능하도록 설정
	TileMesh->SetGenerateOverlapEvents(true);
	TileMesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	TileMesh->SetCollisionResponseToAllChannels(ECR_Block);
	TileMesh->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block); // 클릭 감지

	// 클릭 이벤트 바인딩
	TileMesh->OnClicked.AddDynamic(this, &AWorldTileActor::OnTileClicked);

	// 숲 메시 컴포넌트 생성 (루트 씬 컴포넌트에 직접 부착)
	ForestMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ForestMesh"));
	ForestMesh->SetupAttachment(RootSceneComponent);
	ForestMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 숲은 콜리전 없음
	ForestMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f)); // 높이는 UpdateVisual()에서 동적 설정

	// 자원 메시 컴포넌트 생성 (루트 씬 컴포넌트에 직접 부착)
	ResourceMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ResourceMesh"));
	ResourceMesh->SetupAttachment(RootSceneComponent);
	ResourceMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 자원은 콜리전 없음
	ResourceMesh->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f)); // 높이는 UpdateVisual()에서 동적 설정

	// 기본값 초기화
	TileData = nullptr;
	bIsSelected = false;

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
}

void AWorldTileActor::BeginPlay()
{
	Super::BeginPlay();
	
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
	if (!TileData)
	{
		return;
	}

	// 지형 타입에 따라 메시 스케일 결정
	ELandType LandType = TileData->GetLandType();
	FVector MeshScale = FVector(1.0f, 1.0f, 1.0f);
	float ForestZOffset = 73.0f; // 평지 기본값
	
	if (LandType == ELandType::Hills)
	{
		MeshScale.Z = 2.0f; // 언덕은 Z축 스케일 2.0
		ForestZOffset = 146.0f;
	}
	else if (LandType == ELandType::Mountains)
	{
		MeshScale.Z = 3.0f; // 산은 Z축 스케일 3.0
		ForestZOffset = 219.0f;
	}

	// 지형 타입에 따라 메시 변경
	switch (TileData->GetTerrainType())
	{
	case ETerrainType::Land:
		{
		// 기후대에 따라 메시 선택 (데이터테이블에서 가져오기)
		EClimateType ClimateType = TileData->GetClimateType();

		// 데이터테이블에서 메시 가져오기
		UStaticMesh* SelectedMesh = GetClimateTileMesh(ClimateType);

			// 메시 적용 및 스케일 조정 (메시 자체의 머티리얼 사용)
			if (SelectedMesh && TileMesh)
			{
				TileMesh->SetStaticMesh(SelectedMesh);
				TileMesh->SetRelativeScale3D(MeshScale);
			}
		}
		break;
		
	case ETerrainType::Ocean:
		// 바다 타일 - 메시 설정 (메시 자체의 머티리얼 사용)
		if (OceanMesh && TileMesh)
		{
			TileMesh->SetStaticMesh(OceanMesh);
			TileMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f)); // 바다는 스케일 1.0
		}
		break;
		
	default:
		break;
	}

	// 숲 메시 처리
	if (TileData->HasForest() && ForestMesh)
	{
		// 기후대에 따라 숲 메시 선택 (데이터테이블에서 가져오기)
		EClimateType ClimateType = TileData->GetClimateType();

		// 데이터테이블에서 숲 메시 가져오기
		UStaticMesh* SelectedForestMesh = GetClimateForestMesh(ClimateType);

		// 숲 메시 적용 및 지형에 따른 높이 조정
		if (SelectedForestMesh)
		{
			ForestMesh->SetStaticMesh(SelectedForestMesh);
			ForestMesh->SetRelativeLocation(FVector(0.0f, 0.0f, ForestZOffset));
			ForestMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f)); // 숲은 항상 스케일 1.0
			ForestMesh->SetVisibility(true);
		}
	}
	else if (ForestMesh)
	{
		// 숲이 없으면 숲 메시 숨김
		ForestMesh->SetVisibility(false);
	}

	// 자원 메시 처리 (숲과 동일한 방식)
	if (TileData->HasResource() && ResourceMesh)
	{
		// 지형 높이에 따른 Z 오프셋 계산 (숲과 동일)
		float ResourceZOffset = 73.0f; // 평지 기본값
		
		if (LandType == ELandType::Hills)
		{
			ResourceZOffset = 146.0f;
		}
		else if (LandType == ELandType::Mountains)
		{
			ResourceZOffset = 219.0f;
		}

		// 자원 타입에 따라 메시 선택
		UStaticMesh* SelectedResourceMesh = nullptr;
		
		switch (TileData->GetResourceCategory())
		{
		case EResourceCategory::Bonus:
			// 보너스 자원은 데이터테이블에서 메시 가져오기
			SelectedResourceMesh = GetBonusResourceMesh(TileData->GetBonusResource());
			break;
			
		case EResourceCategory::Strategic:
			// 전략 자원은 데이터테이블에서 메시 가져오기
			SelectedResourceMesh = GetStrategicResourceMesh(TileData->GetStrategicResource());
			break;
			
		case EResourceCategory::Luxury:
			// 사치 자원은 데이터테이블에서 메시 가져오기
			SelectedResourceMesh = GetLuxuryResourceMesh(TileData->GetLuxuryResource());
			break;
		}

		// 자원 메시 적용 및 지형에 따른 높이 조정
		if (SelectedResourceMesh)
		{
			ResourceMesh->SetStaticMesh(SelectedResourceMesh);
			ResourceMesh->SetRelativeLocation(FVector(0.0f, 0.0f, ResourceZOffset));
			ResourceMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f)); // 자원은 항상 스케일 1.0
			ResourceMesh->SetVisibility(true);
		}
		else
		{
			ResourceMesh->SetVisibility(false);
		}
	}
	else if (ResourceMesh)
	{
		// 자원이 없으면 자원 메시 숨김
		ResourceMesh->SetVisibility(false);
	}

}

UStaticMesh* AWorldTileActor::GetStrategicResourceMesh(EStrategicResource Resource) const
{
	// GameInstance에서 WorldComponent 가져오기
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
			{
				if (UDataTable* StrategicResourceDataTable = WorldComponent->GetStrategicResourceDataTable())
				{
					// Enum 값을 문자열로 변환
					FString EnumString = UEnum::GetValueAsString(Resource);
					FString ResourceName = EnumString.RightChop(EnumString.Find(TEXT("::")) + 2);
					
					// 데이터테이블에서 해당 자원 데이터 찾기
					FStrategicResourceData* ResourceData = StrategicResourceDataTable->FindRow<FStrategicResourceData>(FName(*ResourceName), TEXT("StrategicResourceData"));
					if (ResourceData && ResourceData->ResourceMesh)
					{
						return ResourceData->ResourceMesh;
					}
				}
			}
		}
	}
	
	return nullptr;
}

UStaticMesh* AWorldTileActor::GetBonusResourceMesh(EBonusResource Resource) const
{
	// GameInstance에서 WorldComponent 가져오기
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
			{
				if (UDataTable* BonusResourceDataTable = WorldComponent->GetBonusResourceDataTable())
				{
					// Enum 값을 문자열로 변환
					FString EnumString = UEnum::GetValueAsString(Resource);
					FString ResourceName = EnumString.RightChop(EnumString.Find(TEXT("::")) + 2);
					
					// 데이터테이블에서 해당 자원 데이터 찾기
					FBonusResourceData* ResourceData = BonusResourceDataTable->FindRow<FBonusResourceData>(FName(*ResourceName), TEXT("BonusResourceData"));
					if (ResourceData && ResourceData->ResourceMesh)
					{
						return ResourceData->ResourceMesh;
					}
				}
			}
		}
	}
	
	return nullptr;
}

UStaticMesh* AWorldTileActor::GetLuxuryResourceMesh(ELuxuryResource Resource) const
{
	// GameInstance에서 WorldComponent 가져오기
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
			{
				if (UDataTable* LuxuryResourceDataTable = WorldComponent->GetLuxuryResourceDataTable())
				{
					// Enum 값을 문자열로 변환
					FString EnumString = UEnum::GetValueAsString(Resource);
					FString ResourceName = EnumString.RightChop(EnumString.Find(TEXT("::")) + 2);
					
					// 데이터테이블에서 해당 자원 데이터 찾기
					FLuxuryResourceData* ResourceData = LuxuryResourceDataTable->FindRow<FLuxuryResourceData>(FName(*ResourceName), TEXT("LuxuryResourceData"));
					if (ResourceData && ResourceData->ResourceMesh)
					{
						return ResourceData->ResourceMesh;
					}
				}
			}
		}
	}
	
	return nullptr;
}

UStaticMesh* AWorldTileActor::GetClimateTileMesh(EClimateType Climate) const
{
	// GameInstance에서 WorldComponent 가져오기
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
			{
				if (UDataTable* ClimateDataTable = WorldComponent->GetClimateDataTable())
				{
					// Enum 값을 문자열로 변환
					FString EnumString = UEnum::GetValueAsString(Climate);
					FString ClimateName = EnumString.RightChop(EnumString.Find(TEXT("::")) + 2);
					
					// 데이터테이블에서 해당 기후 데이터 찾기
					FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(FName(*ClimateName), TEXT("ClimateData"));
					if (ClimateData && ClimateData->TileMesh)
					{
						return ClimateData->TileMesh;
					}
				}
			}
		}
	}
	
	return nullptr;
}

UStaticMesh* AWorldTileActor::GetClimateForestMesh(EClimateType Climate) const
{
	// GameInstance에서 WorldComponent 가져오기
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
			{
				if (UDataTable* ClimateDataTable = WorldComponent->GetClimateDataTable())
				{
					// Enum 값을 문자열로 변환
					FString EnumString = UEnum::GetValueAsString(Climate);
					FString ClimateName = EnumString.RightChop(EnumString.Find(TEXT("::")) + 2);
					
					// 데이터테이블에서 해당 기후 데이터 찾기
					FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(FName(*ClimateName), TEXT("ClimateData"));
					if (ClimateData && ClimateData->ForestMesh)
					{
						return ClimateData->ForestMesh;
					}
				}
			}
		}
	}
	
	return nullptr;
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
			
			UE_LOG(LogTemp, Warning, TEXT("===== 타일 클릭 ====="));
			UE_LOG(LogTemp, Warning, TEXT("육각형 좌표: Q=%d, R=%d"), (int32)HexPos.X, (int32)HexPos.Y);
			UE_LOG(LogTemp, Warning, TEXT("월드 좌표: X=%.1f, Y=%.1f, Z=%.1f"), WorldPos.X, WorldPos.Y, WorldPos.Z);
			
			// 바다 타일인지 확인
			if (TileData->GetTerrainType() == ETerrainType::Ocean)
			{
				UE_LOG(LogTemp, Warning, TEXT("타입: 바다"));
				UE_LOG(LogTemp, Warning, TEXT("===================="));
			}
			else
			{
				// 육지 타일의 경우 기존 상세 정보 출력
				FString ClimateName = TileData->GetClimateTypeName();
				FString LandTypeName = TileData->GetLandTypeName();
				
				UE_LOG(LogTemp, Warning, TEXT("기후대: %s"), *ClimateName);
				UE_LOG(LogTemp, Warning, TEXT("지형: %s"), *LandTypeName);
				
				// 자원 정보 출력
				if (TileData->HasResource())
				{
					FString ResourceName;
					switch (TileData->GetResourceCategory())
					{
					case EResourceCategory::Bonus:
						ResourceName = TileData->GetBonusResourceName();
						UE_LOG(LogTemp, Warning, TEXT("보너스 자원: %s"), *ResourceName);
						break;
					case EResourceCategory::Strategic:
						ResourceName = TileData->GetStrategicResourceName();
						UE_LOG(LogTemp, Warning, TEXT("전략 자원: %s"), *ResourceName);
						break;
					case EResourceCategory::Luxury:
						ResourceName = TileData->GetLuxuryResourceName();
						UE_LOG(LogTemp, Warning, TEXT("사치 자원: %s"), *ResourceName);
						break;
					}
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("자원: 없음"));
				}
				
				// 숲 정보 출력
				if (TileData->HasForest())
				{
					UE_LOG(LogTemp, Warning, TEXT("숲: 있음"));
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("숲: 없음"));
				}
				
				// 이동 비용 정보 출력
				int32 MovementCost = TileData->GetTotalMovementCost();
				UE_LOG(LogTemp, Warning, TEXT("이동 비용: %d"), MovementCost);
				
				UE_LOG(LogTemp, Warning, TEXT("===================="));
			}
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

