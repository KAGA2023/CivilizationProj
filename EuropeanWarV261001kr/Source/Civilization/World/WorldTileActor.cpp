// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldTileActor.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/WidgetComponent.h"
#include "Engine/Engine.h"
#include "UObject/ConstructorHelpers.h"
#include "WorldComponent.h"
#include "../SuperGameInstance.h"
#include "../SuperGameModeBase.h"
#include "../Unit/UnitManager.h"
#include "../Unit/UnitCharacterBase.h"
#include "../SuperPlayerState.h"
#include "../Turn/TurnComponent.h"
#include "../Widget/TileWidget/TileUI.h"

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

	// 호버 이벤트 바인딩
	TileMesh->OnBeginCursorOver.AddDynamic(this, &AWorldTileActor::OnBeginCursorOver);
	TileMesh->OnEndCursorOver.AddDynamic(this, &AWorldTileActor::OnEndCursorOver);

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

	// 타일 위젯 컴포넌트 생성 (루트 씬 컴포넌트에 부착)
	TileWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("TileWidget"));
	TileWidget->SetupAttachment(RootSceneComponent);
	TileWidget->SetWidgetSpace(EWidgetSpace::World); // World Space로 설정 (카메라 줌/이동 영향받음)
	TileWidget->SetCollisionEnabled(ECollisionEnabled::NoCollision); // 위젯은 콜리전 없음
	TileWidget->SetRelativeLocation(FVector(0.0f, 0.0f, 0.0f)); // 높이는 UpdateVisual()에서 동적 설정
	TileWidget->SetRelativeRotation(FRotator(90.0f, 180.0f, 0.0f)); // Y축 90도, Z축 180도 회전
	TileWidget->SetRelativeScale3D(FVector(0.75f, 0.75f, 0.75f)); // 위젯 스케일 0.75배
	
	// 위젯 클래스 로드 및 설정
	static ConstructorHelpers::FClassFinder<UUserWidget> TileWidgetClassFinder(TEXT("/Game/Civilization/Widget/TileWidget/W_Tile"));
	if (TileWidgetClassFinder.Succeeded())
	{
		TileWidget->SetWidgetClass(TileWidgetClassFinder.Class);
	}
	
	// 초기에는 위젯 숨김
	TileWidget->SetVisibility(false);

	// Custom Depth 활성화 (밝기 조절용)
	TileMesh->SetRenderCustomDepth(true);
	TileMesh->SetCustomDepthStencilValue(0); // 기본: 보통 밝기
	
	ForestMesh->SetRenderCustomDepth(true);
	ForestMesh->SetCustomDepthStencilValue(0);
	
	ResourceMesh->SetRenderCustomDepth(true);
	ResourceMesh->SetCustomDepthStencilValue(0);

	// 기본값 초기화
	TileData = nullptr;
	bIsSelected = false;
	bIsPurchaseableHighlighted = false;

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

	// 지형 높이에 따른 Z 오프셋 (숲/자원 공통)
	float ResourceZOffset = 73.0f; // 평지 기본값
	if (LandType == ELandType::Hills)
	{
		ResourceZOffset = 146.0f;
	}
	else if (LandType == ELandType::Mountains)
	{
		ResourceZOffset = 219.0f;
	}

	const bool bHasForest = TileData->HasForest();
	const bool bHasResource = TileData->HasResource();

	// 숲+자원 타일: ForestResourceMesh 하나만 ResourceMesh에 표시, ForestMesh 숨김
	if (bHasForest && bHasResource && ResourceMesh)
	{
		UStaticMesh* SelectedForestResourceMesh = nullptr;
		switch (TileData->GetResourceCategory())
		{
		case EResourceCategory::Bonus:
			SelectedForestResourceMesh = GetBonusForestResourceMesh(TileData->GetBonusResource());
			break;
		case EResourceCategory::Strategic:
			SelectedForestResourceMesh = GetStrategicForestResourceMesh(TileData->GetStrategicResource());
			break;
		case EResourceCategory::Luxury:
			SelectedForestResourceMesh = GetLuxuryForestResourceMesh(TileData->GetLuxuryResource());
			break;
		default:
			break;
		}
		if (ForestMesh)
		{
			ForestMesh->SetVisibility(false);
		}
		if (SelectedForestResourceMesh)
		{
			ResourceMesh->SetStaticMesh(SelectedForestResourceMesh);
			ResourceMesh->SetRelativeLocation(FVector(0.0f, 0.0f, ResourceZOffset));
			ResourceMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
			ResourceMesh->SetVisibility(true);
		}
		else
		{
			ResourceMesh->SetVisibility(false);
		}
	}
	// 숲만 있는 타일: FClimateData ForestMesh 사용
	else if (bHasForest && ForestMesh)
	{
		EClimateType ClimateType = TileData->GetClimateType();
		UStaticMesh* SelectedForestMesh = GetClimateForestMesh(ClimateType);
		if (SelectedForestMesh)
		{
			ForestMesh->SetStaticMesh(SelectedForestMesh);
			ForestMesh->SetRelativeLocation(FVector(0.0f, 0.0f, ForestZOffset));
			ForestMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
			ForestMesh->SetVisibility(true);
		}
		if (ResourceMesh)
		{
			ResourceMesh->SetVisibility(false);
		}
	}
	// 자원만 있는 타일: ResourceMesh 사용
	else if (bHasResource && ResourceMesh)
	{
		UStaticMesh* SelectedResourceMesh = nullptr;
		switch (TileData->GetResourceCategory())
		{
		case EResourceCategory::Bonus:
			SelectedResourceMesh = GetBonusResourceMesh(TileData->GetBonusResource());
			break;
		case EResourceCategory::Strategic:
			SelectedResourceMesh = GetStrategicResourceMesh(TileData->GetStrategicResource());
			break;
		case EResourceCategory::Luxury:
			SelectedResourceMesh = GetLuxuryResourceMesh(TileData->GetLuxuryResource());
			break;
		default:
			break;
		}
		if (ForestMesh)
		{
			ForestMesh->SetVisibility(false);
		}
		if (SelectedResourceMesh)
		{
			ResourceMesh->SetStaticMesh(SelectedResourceMesh);
			ResourceMesh->SetRelativeLocation(FVector(0.0f, 0.0f, ResourceZOffset));
			ResourceMesh->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));
			ResourceMesh->SetVisibility(true);
		}
		else
		{
			ResourceMesh->SetVisibility(false);
		}
	}
	else
	{
		if (ForestMesh)
		{
			ForestMesh->SetVisibility(false);
		}
		if (ResourceMesh)
		{
			ResourceMesh->SetVisibility(false);
		}
	}

	// 위젯 컴포넌트 지형 높이에 따른 Z 오프셋 처리
	if (TileWidget)
	{
		// 지형 높이에 따른 Z 오프셋 계산 (UnitManager의 유닛 위치와 동일한 높이)
		float WidgetZOffset = 74.0f; // 평지 기본값
		
		switch (LandType)
		{
		case ELandType::Plains:
			WidgetZOffset = 74.0f; // 평지
			break;
		case ELandType::Hills:
			WidgetZOffset = 148.0f; // 언덕
			break;
		case ELandType::Mountains:
			WidgetZOffset = 222.0f; // 산
			break;
		default:
			WidgetZOffset = 74.0f; // 기본값 (평지)
			break;
		}

		// 위젯 위치 업데이트 (X, Y는 유지하고 Z만 변경)
		FVector CurrentLocation = TileWidget->GetRelativeLocation();
		TileWidget->SetRelativeLocation(FVector(CurrentLocation.X, CurrentLocation.Y, WidgetZOffset));
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

UStaticMesh* AWorldTileActor::GetBonusForestResourceMesh(EBonusResource Resource) const
{
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
			{
				if (UDataTable* BonusResourceDataTable = WorldComponent->GetBonusResourceDataTable())
				{
					FString EnumString = UEnum::GetValueAsString(Resource);
					FString ResourceName = EnumString.RightChop(EnumString.Find(TEXT("::")) + 2);
					FBonusResourceData* ResourceData = BonusResourceDataTable->FindRow<FBonusResourceData>(FName(*ResourceName), TEXT("BonusResourceData"));
					if (ResourceData && ResourceData->ForestResourceMesh)
					{
						return ResourceData->ForestResourceMesh;
					}
				}
			}
		}
	}
	return nullptr;
}

UStaticMesh* AWorldTileActor::GetStrategicForestResourceMesh(EStrategicResource Resource) const
{
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
			{
				if (UDataTable* StrategicResourceDataTable = WorldComponent->GetStrategicResourceDataTable())
				{
					FString EnumString = UEnum::GetValueAsString(Resource);
					FString ResourceName = EnumString.RightChop(EnumString.Find(TEXT("::")) + 2);
					FStrategicResourceData* ResourceData = StrategicResourceDataTable->FindRow<FStrategicResourceData>(FName(*ResourceName), TEXT("StrategicResourceData"));
					if (ResourceData && ResourceData->ForestResourceMesh)
					{
						return ResourceData->ForestResourceMesh;
					}
				}
			}
		}
	}
	return nullptr;
}

UStaticMesh* AWorldTileActor::GetLuxuryForestResourceMesh(ELuxuryResource Resource) const
{
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
			{
				if (UDataTable* LuxuryResourceDataTable = WorldComponent->GetLuxuryResourceDataTable())
				{
					FString EnumString = UEnum::GetValueAsString(Resource);
					FString ResourceName = EnumString.RightChop(EnumString.Find(TEXT("::")) + 2);
					FLuxuryResourceData* ResourceData = LuxuryResourceDataTable->FindRow<FLuxuryResourceData>(FName(*ResourceName), TEXT("LuxuryResourceData"));
					if (ResourceData && ResourceData->ForestResourceMesh)
					{
						return ResourceData->ForestResourceMesh;
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

void AWorldTileActor::SetResourceVisibility(bool bVisible)
{
	if (ResourceMesh)
	{
		ResourceMesh->SetVisibility(bVisible);
	}
}

void AWorldTileActor::SetForestVisibility(bool bVisible)
{
	if (ForestMesh)
	{
		ForestMesh->SetVisibility(bVisible);
	}
}

// Custom Depth Stencil 제어 함수들
void AWorldTileActor::SetTileBrightness(int32 StencilValue)
{
	// 모든 메시에 Custom Depth 활성화 및 Stencil 값 적용
	if (TileMesh)
	{
		TileMesh->SetRenderCustomDepth(true);
		TileMesh->SetCustomDepthStencilValue(StencilValue);
	}
	
	if (ForestMesh && ForestMesh->IsVisible())
	{
		ForestMesh->SetRenderCustomDepth(true);
		ForestMesh->SetCustomDepthStencilValue(StencilValue);
	}
	
	if (ResourceMesh && ResourceMesh->IsVisible())
	{
		ResourceMesh->SetRenderCustomDepth(true);
		ResourceMesh->SetCustomDepthStencilValue(StencilValue);
	}
}

void AWorldTileActor::EnableCustomDepth(bool bEnable)
{
	// 모든 메시의 Custom Depth 렌더링 활성화/비활성화
	if (TileMesh)
	{
		TileMesh->SetRenderCustomDepth(bEnable);
	}
	
	if (ForestMesh)
	{
		ForestMesh->SetRenderCustomDepth(bEnable);
	}
	
	if (ResourceMesh)
	{
		ResourceMesh->SetRenderCustomDepth(bEnable);
	}
}

void AWorldTileActor::ResetBrightness()
{
	// 밝기 초기화 (보통 밝기)
	SetTileBrightness(0);
}

UWorldComponent* AWorldTileActor::GetWorldComponent() const
{
	// GameInstance에서 WorldComponent 가져오기
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			return SuperGameInst->GetGeneratedWorldComponent();
		}
	}
	
	return nullptr;
}

void AWorldTileActor::OnTileClicked(UPrimitiveComponent* TouchedComponent, FKey ButtonPressed)
{
	// 좌클릭만 처리
	if (ButtonPressed == EKeys::LeftMouseButton)
	{
		// 턴 체크: 플레이어 0의 턴이 아니면 클릭 무시
		if (UWorld* World = GetWorld())
		{
			if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
			{
				if (UTurnComponent* TurnComponent = GameMode->GetTurnComponent())
				{
					// 플레이어 0의 턴이 아니면 클릭 무시
					if (TurnComponent->GetCurrentPlayerIndex() != 0)
					{
						return;
					}
				}
			}
		}

		if (!TileData || !GetWorld())
		{
			return;
		}

		FVector2D HexPos = TileData->GetGridPosition();
		bool bIsCityTile = false;
		bool bIsBuilderTile = false;

		if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance()))
		{
			ASuperPlayerState* PlayerState0 = SuperGameInst->GetPlayerState(0);
			UWorldComponent* WorldComp = SuperGameInst->GetGeneratedWorldComponent();
			UUnitManager* UnitManager = SuperGameInst->GetUnitManager();

			if (!PlayerState0 || !WorldComp || !UnitManager)
			{
				return;
			}

			// 도시 타일인지 확인
			if (PlayerState0->HasCity())
			{
				FVector2D CityPos = PlayerState0->GetCityCoordinate();
				if (HexPos == CityPos)
				{
					bIsCityTile = true;
					// 플레이어 0의 도시 타일 클릭 델리게이트 브로드캐스트
					OnPlayerCityTileClicked.Broadcast();
				}
			}

			// 건설자 타일인지 확인 (도시 타일이 아닌 경우에만)
			if (!bIsCityTile)
			{
				const bool bIsOwnedByPlayer0 = PlayerState0->IsTileOwned(HexPos);
				const bool bIsCityTileCheck = WorldComp->IsCityAtHex(HexPos);
				AUnitCharacterBase* UnitAtTile = UnitManager->GetUnitAtHex(HexPos);
				
				if (UnitAtTile && bIsOwnedByPlayer0 && !bIsCityTileCheck)
				{
					if (UnitManager->IsBuilderUnit(UnitAtTile) && UnitAtTile->GetPlayerIndex() == 0)
					{
						bIsBuilderTile = true;
						OnBuilderTileClicked.Broadcast(TileData, HexPos);
					}
				}
			}

			// 일반 타일 클릭 감지 (도시 타일도 아니고 건설자 타일도 아닌 경우)
			if (!bIsCityTile && !bIsBuilderTile)
			{
				OnGeneralTileClicked.Broadcast(HexPos);
			}
		}

		// 타일 구매 모드 중에는 유닛 선택/이동/전투 처리하지 않음 (구매 가능 타일 클릭 시 구매만 수행)
		if (UWorld* World = GetWorld())
		{
			if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
			{
				if (SuperGameInst->IsTilePurchaseMode())
				{
					return;
				}
			}
		}

		// UnitManager에 클릭 전달 (유닛 이동용 2단계 선택)
		if (UWorld* World = GetWorld())
		{
			if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
			{
				if (UUnitManager* UnitManager = SuperGameInst->GetUnitManager())
				{
					// 첫 번째 선택 단계인 경우, 해당 타일의 유닛이 플레이어 0의 유닛인지 확인
					if (!UnitManager->HasMoveFirstSelection())
					{
						AUnitCharacterBase* UnitAtTile = UnitManager->GetUnitAtHex(HexPos);
						
						// 유닛이 있고, 플레이어 0의 유닛이 아니면 이동 선택 건너뛰기
						// (전투 선택은 계속 진행)
						if (UnitAtTile && UnitAtTile->GetPlayerIndex() == 0)
						{
							// ========== 유닛 선택 표시 ==========
							UnitManager->SetSelectedUnit(UnitAtTile);
							// =====================================
							
							UnitManager->HandleMoveSelection(TileData);
							
							// ========== 이동 가능 타일 계산 및 저장 ==========
							UnitManager->CalculateAndStoreReachableTiles(UnitAtTile);
							// =================================================
							
							// ========== 이동 범위 밝기 표시 ==========
							UnitManager->ShowMovementRangeWithBrightness(UnitAtTile);
							// =========================================
						}
					}
					else
					{
						// 이미 첫 번째 이동 선택이 있는 경우
						// 클릭한 타일에 플레이어 0의 유닛이 있는지 확인
						AUnitCharacterBase* UnitAtTile = UnitManager->GetUnitAtHex(HexPos);
						
						if (UnitAtTile && UnitAtTile->GetPlayerIndex() == 0)
						{
							// 플레이어 유닛을 클릭했으면, 이전 선택을 취소하고 새로운 첫 번째 선택으로 처리
							UnitManager->ClearMoveSelection();
							UnitManager->SetSelectedUnit(UnitAtTile);
							UnitManager->HandleMoveSelection(TileData);
							
							// ========== 이동 가능 타일 계산 및 저장 ==========
							UnitManager->CalculateAndStoreReachableTiles(UnitAtTile);
							// =================================================
							
							// ========== 이동 범위 밝기 표시 ==========
							UnitManager->ShowMovementRangeWithBrightness(UnitAtTile);
							// =========================================
						}
						else
						{
							// 빈 타일이나 적 유닛을 클릭했으면 두 번째 선택으로 처리
							UnitManager->HandleMoveSelection(TileData);
						}
					}
				}
			}
		}

		// UnitManager에 클릭 전달 (전투용 2단계 선택) - 둘 다 호출
		if (UWorld* World = GetWorld())
		{
			if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
			{
				if (UUnitManager* UnitManager = SuperGameInst->GetUnitManager())
				{
					// 첫 번째 전투 선택 단계인 경우, 해당 타일의 유닛이 플레이어 0의 유닛인지 확인
					if (!UnitManager->HasCombatFirstSelection())
					{
						AUnitCharacterBase* UnitAtTile = UnitManager->GetUnitAtHex(HexPos);
						
						// 유닛이 있고, 플레이어 0의 유닛이 아니면 전투 선택 건너뛰기
						if (UnitAtTile && UnitAtTile->GetPlayerIndex() != 0)
						{
							// 첫 번째 전투 선택은 플레이어 0의 유닛만 가능
							// return하지 않고 그냥 건너뛰기 (이동 선택은 이미 처리됨)
						}
						else if (UnitAtTile && UnitAtTile->GetPlayerIndex() == 0)
						{
							// ========== 유닛 선택 표시 ==========
							UnitManager->SetSelectedUnit(UnitAtTile);
							// =====================================
							
							// 플레이어 0의 유닛이면 전투 선택 처리
							UnitManager->HandleCombatSelection(TileData);
							
							// ========== 이동 범위 밝기 표시 ==========
							UnitManager->ShowMovementRangeWithBrightness(UnitAtTile);
							// =========================================
						}
						else
						{
							// 유닛이 없으면 전투 선택 처리
							UnitManager->HandleCombatSelection(TileData);
						}
					}
					else
					{
						// 이미 첫 번째 전투 선택이 있는 경우
						// 클릭한 타일에 플레이어 0의 유닛이 있는지 확인
						AUnitCharacterBase* UnitAtTile = UnitManager->GetUnitAtHex(HexPos);
						
						if (UnitAtTile && UnitAtTile->GetPlayerIndex() == 0)
						{
							// 플레이어 유닛을 클릭했으면, 이전 선택을 취소하고 새로운 첫 번째 선택으로 처리
							UnitManager->ClearCombatSelection();
							UnitManager->SetSelectedUnit(UnitAtTile);
							UnitManager->HandleCombatSelection(TileData);
							
							// ========== 이동 범위 밝기 표시 ==========
							UnitManager->ShowMovementRangeWithBrightness(UnitAtTile);
							// =========================================
						}
						else
						{
							// 적 유닛이나 도시를 클릭했으면 두 번째 선택으로 처리 (전투 실행)
							UnitManager->HandleCombatSelection(TileData);
						}
					}
				}
			}
		}
	}
}

void AWorldTileActor::OnBeginCursorOver(UPrimitiveComponent* TouchedComponent)
{
	// WorldComponent에 호버 시작 전달
	if (UWorldComponent* WorldComp = GetWorldComponent())
	{
		WorldComp->HandleTileHoverBegin(TileData);
	}
	
	// 전투 호버 델리게이트 브로드캐스트
	OnCombatTileHoverBegin.Broadcast(TileData);
	
	// 타일 호버 시작 델리게이트 브로드캐스트
	OnTileHoverBegin.Broadcast(TileData);
	
	// 구매 가능한 타일이고 위젯이 표시되어 있을 때 호버 효과 적용
	if (bIsPurchaseableHighlighted && TileWidget && TileWidget->IsVisible())
	{
		if (UTileUI* TileUIWidget = Cast<UTileUI>(TileWidget->GetWidget()))
		{
			TileUIWidget->SetHovered(true);
		}
	}
}

void AWorldTileActor::OnEndCursorOver(UPrimitiveComponent* TouchedComponent)
{
	// WorldComponent에 호버 종료 전달
	if (UWorldComponent* WorldComp = GetWorldComponent())
	{
		WorldComp->HandleTileHoverEnd(TileData);
	}
	
	// 전투 호버 델리게이트 브로드캐스트
	OnCombatTileHoverEnd.Broadcast(TileData);
	
	// 타일 호버 종료 델리게이트 브로드캐스트
	OnTileHoverEnd.Broadcast(TileData);
	
	// 구매 가능한 타일이고 위젯이 표시되어 있을 때 호버 효과 제거
	if (bIsPurchaseableHighlighted && TileWidget && TileWidget->IsVisible())
	{
		if (UTileUI* TileUIWidget = Cast<UTileUI>(TileWidget->GetWidget()))
		{
			TileUIWidget->SetHovered(false);
		}
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

void AWorldTileActor::SetPurchaseableHighlight(bool bHighlight)
{
	bIsPurchaseableHighlighted = bHighlight;
	
	// WidgetComponent의 Visibility로 제어
	if (TileWidget)
	{
		if (bHighlight)
		{
			// 하이라이트 활성화: 위젯 표시
			TileWidget->SetVisibility(true);
			
			// 타일 가격 계산 및 표시
			if (TileData)
			{
				FVector2D TileCoordinate = TileData->GetGridPosition();
				
				// SuperPlayerState와 WorldComponent 가져오기
				if (UWorld* World = GetWorld())
				{
					if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
					{
						ASuperPlayerState* PlayerState = SuperGameInst->GetPlayerState(0);
						UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent();
						
						if (PlayerState && WorldComponent)
						{
							// 타일 가격 계산
							int32 TileCost = PlayerState->CalculateTilePurchaseCost(TileCoordinate, WorldComponent);
							
							// 위젯 가져와서 가격 업데이트
							if (UTileUI* TileUIWidget = Cast<UTileUI>(TileWidget->GetWidget()))
							{
								TileUIWidget->UpdateTileCost(TileCost);
							}
						}
					}
				}
			}
		}
		else
		{
			// 하이라이트 비활성화: 위젯 숨김
			TileWidget->SetVisibility(false);
		}
	}
}

