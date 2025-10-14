// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldComponent.h"
#include "Engine/DataTable.h"

/*
<수정사항들>
1. Generate...() 수정
2. FindPath(), FindPathWithMovementCost() 수정
*/

UWorldComponent::UWorldComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // 기본 월드 설정만 초기화 (임시값 제거)
    WorldConfig = FWorldConfig();
    
    bIsWorldGenerated = false;
    bIsGenerating = false;
    
    // 데이터테이블 로딩
    LoadDataTables();
}

void UWorldComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // 월드 자동 생성 (필요시)
    // GenerateWorld();
}

void UWorldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // 월드 생성 중일 때 처리
    if (bIsGenerating)
    {
        // 비동기 월드 생성 처리
        // TODO: 비동기 월드 생성 로직 구현
    }
}

void UWorldComponent::GenerateWorld()
{
    if (bIsGenerating)
    {
        return; // 이미 생성 중
    }
    
    bIsGenerating = true;
    bIsWorldGenerated = false;
    
    // 기존 월드 초기화
    ClearWorld();
    
    // 육각형 타일들 초기화
    InitializeHexTiles();
    
    // 지형 생성
    GenerateTerrain();
    
    // 기후대 생성
    GenerateClimateZones();
    
    // 땅 타입 생성
    GenerateLandTypes();
    
    // 숲 생성
    GenerateForests();
    
    // 자원 생성
    GenerateResources();
    
    // 모든 타일의 최종 생산량 재계산
    RecalculateAllTileYields();
    
    bIsGenerating = false;
    bIsWorldGenerated = true;
    
    // 월드 생성 완료 이벤트 발생
    OnWorldGenerated.Broadcast(true);
}

void UWorldComponent::ClearWorld()
{
    // 모든 타일 파괴
    for (auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            DestroyTile(Tile);
        }
    }
    
    HexTiles.Empty();
    SelectedTile = nullptr;
    bIsWorldGenerated = false;
}

UWorldTile* UWorldComponent::GetTileAtHex(FVector2D HexPosition) const
{
    if (UWorldTile* const* TilePtr = HexTiles.Find(HexPosition))
    {
        return *TilePtr;
    }
    return nullptr;
}

UWorldTile* UWorldComponent::CreateTileAtHex(FVector2D HexPosition)
{
    if (!IsValidHexPosition(HexPosition))
    {
        return nullptr;
    }
    
    UWorldTile* ExistingTile = GetTileAtHex(HexPosition);
    if (ExistingTile)
    {
        return ExistingTile; // 이미 존재
    }
    
    UWorldTile* NewTile = CreateNewTile(HexPosition);
    if (NewTile)
    {
        HexTiles.Add(HexPosition, NewTile);
    }
    
    return NewTile;
}

bool UWorldComponent::RemoveTileAtHex(FVector2D HexPosition)
{
    if (UWorldTile* Tile = GetTileAtHex(HexPosition))
    {
        HexTiles.Remove(HexPosition);
        if (SelectedTile == Tile)
        {
            SelectedTile = nullptr;
        }
        DestroyTile(Tile);
        return true;
    }
    return false;
}

TArray<UWorldTile*> UWorldComponent::GetAllTiles() const
{
    TArray<UWorldTile*> AllTiles;
    for (const auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            AllTiles.Add(Tile);
        }
    }
    return AllTiles;
}

TArray<UWorldTile*> UWorldComponent::GetTilesInRadius(FVector2D CenterHex, int32 Radius) const
{
    TArray<UWorldTile*> TilesInRadius;
    TArray<FVector2D> HexPositions = GetHexesInRadius(CenterHex, Radius);
    
    for (const FVector2D& HexPos : HexPositions)
    {
        if (UWorldTile* Tile = GetTileAtHex(HexPos))
        {
            TilesInRadius.Add(Tile);
        }
    }
    
    return TilesInRadius;
}

TArray<FVector2D> UWorldComponent::GetHexNeighbors(FVector2D HexPosition) const
{
    TArray<FVector2D> Neighbors;
    
    // Pointy top 육각형의 6방향 인접 타일
    Neighbors.Add(FVector2D(HexPosition.X + 1, HexPosition.Y));     // 오른쪽
    Neighbors.Add(FVector2D(HexPosition.X - 1, HexPosition.Y));     // 왼쪽
    Neighbors.Add(FVector2D(HexPosition.X, HexPosition.Y + 1));     // 오른쪽 아래
    Neighbors.Add(FVector2D(HexPosition.X, HexPosition.Y - 1));     // 왼쪽 위
    Neighbors.Add(FVector2D(HexPosition.X + 1, HexPosition.Y - 1)); // 오른쪽 위
    Neighbors.Add(FVector2D(HexPosition.X - 1, HexPosition.Y + 1)); // 왼쪽 아래
    
    return Neighbors;
}

int32 UWorldComponent::GetHexDistance(FVector2D Hex1, FVector2D Hex2) const
{
    // 육각형 거리 공식: (|q1-q2| + |r1-r2| + |s1-s2|) / 2
    int32 q1 = Hex1.X, r1 = Hex1.Y, s1 = -q1 - r1;
    int32 q2 = Hex2.X, r2 = Hex2.Y, s2 = -q2 - r2;
    
    return (FMath::Abs(q1 - q2) + FMath::Abs(r1 - r2) + FMath::Abs(s1 - s2)) / 2;
}

TArray<FVector2D> UWorldComponent::GetHexesInRadius(FVector2D CenterHex, int32 Radius) const
{
    TArray<FVector2D> HexesInRadius;
    
    for (int32 q = -Radius; q <= Radius; q++)
    {
        int32 r1 = FMath::Max(-Radius, -q - Radius);
        int32 r2 = FMath::Min(Radius, -q + Radius);
        
        for (int32 r = r1; r <= r2; r++)
        {
            FVector2D HexPos = CenterHex + FVector2D(q, r);
            if (IsValidHexPosition(HexPos))
            {
                HexesInRadius.Add(HexPos);
            }
        }
    }
    
    return HexesInRadius;
}

FVector UWorldComponent::HexToWorld(FVector2D HexPosition) const
{
    // Pointy top 육각형을 월드 좌표로 변환
    // 육각형 좌표계: (q, r) → 언리얼 좌표계: (X=가로, Y=세로)
    float hexX = TILE_SIZE * (FMath::Sqrt(3.0f) * HexPosition.X + FMath::Sqrt(3.0f) / 2.0f * HexPosition.Y);
    float hexY = TILE_SIZE * (3.0f / 2.0f * HexPosition.Y);
    
    // 언리얼 좌표계에 맞게 변환: X=가로(Right), Y=앞뒤(Forward)
    return FVector(hexY, hexX, 0.0f);  // X와 Y 바꿈!
}

FVector2D UWorldComponent::WorldToHex(FVector WorldPosition) const
{
    // 언리얼 좌표를 육각형 좌표로 역변환
    // 언리얼: (X=앞뒤, Y=가로) → 육각형: (hexX=가로, hexY=세로)
    float hexX = WorldPosition.Y;  // X와 Y 바꿈
    float hexY = WorldPosition.X;
    
    // Pointy top 육각형 좌표 계산
    float q = (FMath::Sqrt(3.0f) / 3.0f * hexX - 1.0f / 3.0f * hexY) / TILE_SIZE;
    float r = (2.0f / 3.0f * hexY) / TILE_SIZE;
    
    return FVector2D(q, r);
}

bool UWorldComponent::IsValidHexPosition(FVector2D HexPosition) const
{
    // 원형 경계: 중심에서의 거리가 반지름 이내인지 확인
    int32 Distance = GetHexDistance(FVector2D::ZeroVector, HexPosition);
    return Distance <= WorldConfig.WorldRadius;
}

void UWorldComponent::SelectTile(UWorldTile* Tile)
{
    if (!Tile)
    {
        return;
    }
    
    // 기존 선택 해제
    if (SelectedTile)
    {
        SelectedTile->SetSelected(false);
    }
    
    // 새 타일 선택
    SelectedTile = Tile;
    Tile->SetSelected(true);
    
    NotifyTileSelected(Tile);
}

void UWorldComponent::DeselectTile()
{
    if (SelectedTile)
    {
        SelectedTile->SetSelected(false);
        SelectedTile = nullptr;
    }
}

void UWorldComponent::SetWorldConfig(const FWorldConfig& NewSettings)
{
    WorldConfig = NewSettings;
}

void UWorldComponent::LoadDataTables()
{
    // 기후대 데이터테이블 로딩
    if (!ClimateDataTable)
    {
        ClimateDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Civilization/Data/DT_ClimateData.DT_ClimateData"));
    }
    
    // 땅 타입 데이터테이블 로딩
    if (!LandTypeDataTable)
    {
        LandTypeDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Civilization/Data/DT_LandTypeData.DT_LandTypeData"));
    }
}

void UWorldComponent::GenerateTerrain()
{
    // 판게아 스타일 지형 생성 (연결된 대륙)
    GeneratePangaeaTerrain();
}

void UWorldComponent::GenerateResources()
{
    // 기본 자원 생성 (나중에 복잡한 알고리즘으로 변경)
    for (auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            // 임시로 랜덤 자원 생성
            float RandomValue = FMath::FRand();
            
            if (RandomValue < 0.1f) // 10% 확률로 자원 생성
            {
                // 보너스 자원 생성
                Tile->SetResourceCategory(EResourceCategory::Bonus);
                Tile->SetBonusResource(static_cast<EBonusResource>(FMath::RandRange(1, 9))); // 1~9 (None 제외)
            }
        }
    }
}

void UWorldComponent::GenerateClimateZones()
{
    // 기본 기후대 생성 (나중에 복잡한 알고리즘으로 변경)
    for (auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetTerrainType() == ETerrainType::Land)
            {
                // 임시로 랜덤 기후대 생성
                float RandomValue = FMath::FRand();
                
                if (RandomValue < WorldConfig.TemperatePercentage)
                {
                    Tile->SetClimateTypeID("Temperate");
                }
                else if (RandomValue < WorldConfig.TemperatePercentage + WorldConfig.DesertPercentage)
                {
                    Tile->SetClimateTypeID("Desert");
                }
                else if (RandomValue < WorldConfig.TemperatePercentage + WorldConfig.DesertPercentage + WorldConfig.TundraPercentage)
                {
                    Tile->SetClimateTypeID("Tundra");
                }
                else
                {
                    // 기본값으로 온대 기후 설정
                    Tile->SetClimateTypeID("Temperate");
                }
            }
        }
    }
}

void UWorldComponent::GenerateLandTypes()
{
    // 기본 땅 타입 생성 (나중에 복잡한 알고리즘으로 변경)
    for (auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetTerrainType() == ETerrainType::Land)
            {
                // 임시로 랜덤 땅 타입 생성
                float RandomValue = FMath::FRand();
                
                if (RandomValue < WorldConfig.PlainsPercentage)
                {
                    Tile->SetLandTypeID("Plains");
                }
                else if (RandomValue < WorldConfig.PlainsPercentage + WorldConfig.HillsPercentage)
                {
                    Tile->SetLandTypeID("Hills");
                }
                else
                {
                    Tile->SetLandTypeID("Mountains");
                }
            }
        }
    }
}

void UWorldComponent::GenerateForests()
{
    // 숲 생성 (땅 타일에만 적용)
    for (auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetTerrainType() == ETerrainType::Land)
            {
                // 랜덤으로 숲 생성
                float RandomValue = FMath::FRand();
                if (RandomValue < WorldConfig.ForestPercentage)
                {
                    Tile->SetHasForest(true);
                }
            }
        }
    }
}


int32 UWorldComponent::GetLandTileCount() const
{
    int32 Count = 0;
    for (const auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetTerrainType() == ETerrainType::Land)
            {
                Count++;
            }
        }
    }
    return Count;
}

int32 UWorldComponent::GetOceanTileCount() const
{
    int32 Count = 0;
    for (const auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetTerrainType() == ETerrainType::Ocean)
            {
                Count++;
            }
        }
    }
    return Count;
}

int32 UWorldComponent::GetResourceTileCount(EResourceCategory ResourceCategory) const
{
    int32 Count = 0;
    for (const auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetResourceCategory() == ResourceCategory)
            {
                Count++;
            }
        }
    }
    return Count;
}

int32 UWorldComponent::GetClimateTileCount(EClimateType ClimateType) const
{
    // Enum을 FName으로 변환
    FName ClimateTypeName;
    switch (ClimateType)
    {
    case EClimateType::Temperate:
        ClimateTypeName = FName("Temperate");
        break;
    case EClimateType::Desert:
        ClimateTypeName = FName("Desert");
        break;
    case EClimateType::Tundra:
        ClimateTypeName = FName("Tundra");
        break;
    default:
        ClimateTypeName = NAME_None;
        break;
    }
    
    int32 Count = 0;
    for (const auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetClimateTypeID() == ClimateTypeName)
            {
                Count++;
            }
        }
    }
    return Count;
}

int32 UWorldComponent::GetLandTypeTileCount(ELandType LandType) const
{
    // Enum을 FName으로 변환
    FName LandTypeName;
    switch (LandType)
    {
    case ELandType::Plains:
        LandTypeName = FName("Plains");
        break;
    case ELandType::Hills:
        LandTypeName = FName("Hills");
        break;
    case ELandType::Mountains:
        LandTypeName = FName("Mountains");
        break;
    default:
        LandTypeName = NAME_None;
        break;
    }
    
    int32 Count = 0;
    for (const auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetLandTypeID() == LandTypeName)
            {
                Count++;
            }
        }
    }
    return Count;
}

int32 UWorldComponent::GetForestTileCount() const
{
    int32 Count = 0;
    for (const auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->HasForest())
            {
                Count++;
            }
        }
    }
    return Count;
}


TArray<FVector2D> UWorldComponent::FindPath(FVector2D StartHex, FVector2D EndHex) const
{
    // 기본 경로 찾기 (나중에 A* 알고리즘으로 변경)
    TArray<FVector2D> Path;
    
    if (StartHex == EndHex)
    {
        Path.Add(StartHex);
        return Path;
    }
    
    // 임시로 직선 경로 반환
    int32 Distance = GetHexDistance(StartHex, EndHex);
    for (int32 i = 0; i <= Distance; i++)
    {
        float T = (float)i / (float)Distance;
        FVector2D InterpolatedHex = FVector2D(
            FMath::Lerp(StartHex.X, EndHex.X, T),
            FMath::Lerp(StartHex.Y, EndHex.Y, T)
        );
        Path.Add(InterpolatedHex);
    }
    
    return Path;
}

TArray<FVector2D> UWorldComponent::FindPathWithMovementCost(FVector2D StartHex, FVector2D EndHex, float MaxMovementCost) const
{
    // 이동 비용을 고려한 경로 찾기 (나중에 구현)
    return FindPath(StartHex, EndHex);
}

bool UWorldComponent::CanMoveToHex(FVector2D HexPosition) const
{
    UWorldTile* Tile = GetTileAtHex(HexPosition);
    if (!Tile)
    {
        return false;
    }
    
    return Tile->IsPassable();
}

// 내부 헬퍼 함수들
void UWorldComponent::InitializeHexTiles()
{
    // 원형 맵의 육각형 타일들 초기화
    int32 Radius = WorldConfig.WorldRadius;
    
    for (int32 q = -Radius; q <= Radius; q++)
    {
        for (int32 r = -Radius; r <= Radius; r++)
        {
            FVector2D HexPos(q, r);
            if (IsValidHexPosition(HexPos))
            {
                CreateTileAtHex(HexPos);
            }
        }
    }
}

UWorldTile* UWorldComponent::CreateNewTile(FVector2D HexPosition)
{
    UWorldTile* NewTile = NewObject<UWorldTile>(this);
    if (NewTile)
    {
        NewTile->SetGridPosition(HexPosition);
        FVector WorldPos = HexToWorld(HexPosition);
        NewTile->SetWorldPosition(WorldPos);
        
        // 기본 타일 데이터 설정
        FTileData TileData = NewTile->GetTileData();
        TileData.GridPosition = HexPosition;
        TileData.WorldPosition = WorldPos;
        NewTile->SetTileData(TileData);
    }
    return NewTile;
}

void UWorldComponent::DestroyTile(UWorldTile* Tile)
{
    if (Tile)
    {
        Tile->MarkAsGarbage();
    }
}

void UWorldComponent::NotifyTileUpdated(UWorldTile* Tile)
{
    OnTileUpdated.Broadcast(Tile);
}

void UWorldComponent::NotifyTileSelected(UWorldTile* Tile)
{
    OnTileSelected.Broadcast(Tile);
}

// 데이터테이블 기반 계산 함수들 구현
void UWorldComponent::RecalculateTileYields(UWorldTile* Tile)
{
    if (!Tile)
    {
        return;
    }
    
    FTileData TileData = Tile->GetTileData();
    TileData.CachedFoodYield = CalculateBaseFoodYield(Tile);
    TileData.CachedProductionYield = CalculateBaseProductionYield(Tile);
    TileData.CachedGoldYield = CalculateBaseGoldYield(Tile);
    TileData.CachedScienceYield = CalculateBaseScienceYield(Tile);
    TileData.CachedFaithYield = CalculateBaseFaithYield(Tile);
    TileData.CachedMovementCost = CalculateBaseMovementCost(Tile);
    TileData.CachedDefenseBonus = CalculateBaseDefenseBonus(Tile);
    Tile->SetTileData(TileData);
}

void UWorldComponent::RecalculateAllTileYields()
{
    for (auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            RecalculateTileYields(Tile);
        }
    }
}

int32 UWorldComponent::CalculateBaseFoodYield(UWorldTile* Tile) const
{
    if (!Tile)
    {
        return 0;
    }
    
    int32 TotalFood = 0;
    FTileData TileData = Tile->GetTileData();
    
    // 기후대 기본값
    if (ClimateDataTable && !TileData.ClimateTypeID.IsNone())
    {
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(TileData.ClimateTypeID, TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalFood += ClimateData->BaseFoodYield;
        }
    }
    
    // 땅 타입 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land && !TileData.LandTypeID.IsNone())
    {
        if (LandTypeDataTable)
        {
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(TileData.LandTypeID, TEXT("LandTypeData"));
            if (LandData)
            {
                TotalFood += LandData->FoodBonus;
            }
        }
    }
    
    return TotalFood;
}

int32 UWorldComponent::CalculateBaseProductionYield(UWorldTile* Tile) const
{
    if (!Tile)
    {
        return 0;
    }
    
    int32 TotalProduction = 0;
    FTileData TileData = Tile->GetTileData();
    
    // 기후대 기본값
    if (ClimateDataTable && !TileData.ClimateTypeID.IsNone())
    {
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(TileData.ClimateTypeID, TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalProduction += ClimateData->BaseProductionYield;
        }
    }
    
    // 땅 타입 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land && !TileData.LandTypeID.IsNone())
    {
        if (LandTypeDataTable)
        {
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(TileData.LandTypeID, TEXT("LandTypeData"));
            if (LandData)
            {
                TotalProduction += LandData->ProductionBonus;
            }
        }
    }
    
    return TotalProduction;
}

int32 UWorldComponent::CalculateBaseGoldYield(UWorldTile* Tile) const
{
    if (!Tile)
    {
        return 0;
    }
    
    int32 TotalGold = 0;
    FTileData TileData = Tile->GetTileData();
    
    // 기후대 기본값
    if (ClimateDataTable && !TileData.ClimateTypeID.IsNone())
    {
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(TileData.ClimateTypeID, TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalGold += ClimateData->BaseGoldYield;
        }
    }
    
    // 땅 타입 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land && !TileData.LandTypeID.IsNone())
    {
        if (LandTypeDataTable)
        {
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(TileData.LandTypeID, TEXT("LandTypeData"));
            if (LandData)
            {
                TotalGold += LandData->GoldBonus;
            }
        }
    }
    
    return TotalGold;
}

int32 UWorldComponent::CalculateBaseScienceYield(UWorldTile* Tile) const
{
    if (!Tile)
    {
        return 0;
    }
    
    int32 TotalScience = 0;
    FTileData TileData = Tile->GetTileData();
    
    // 기후대 기본값
    if (ClimateDataTable && !TileData.ClimateTypeID.IsNone())
    {
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(TileData.ClimateTypeID, TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalScience += ClimateData->BaseScienceYield;
        }
    }
    
    // 땅 타입 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land && !TileData.LandTypeID.IsNone())
    {
        if (LandTypeDataTable)
        {
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(TileData.LandTypeID, TEXT("LandTypeData"));
            if (LandData)
            {
                TotalScience += LandData->ScienceBonus;
            }
        }
    }
    
    return TotalScience;
}

int32 UWorldComponent::CalculateBaseFaithYield(UWorldTile* Tile) const
{
    if (!Tile)
    {
        return 0;
    }
    
    int32 TotalFaith = 0;
    FTileData TileData = Tile->GetTileData();
    
    // 기후대 기본값
    if (ClimateDataTable && !TileData.ClimateTypeID.IsNone())
    {
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(TileData.ClimateTypeID, TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalFaith += ClimateData->BaseFaithYield;
        }
    }
    
    // 땅 타입 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land && !TileData.LandTypeID.IsNone())
    {
        if (LandTypeDataTable)
        {
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(TileData.LandTypeID, TEXT("LandTypeData"));
            if (LandData)
            {
                TotalFaith += LandData->FaithBonus;
            }
        }
    }
    
    return TotalFaith;
}

float UWorldComponent::CalculateBaseMovementCost(UWorldTile* Tile) const
{
    if (!Tile)
    {
        return 1.0f;
    }
    
    float TotalMovementCost = 1.0f;
    FTileData TileData = Tile->GetTileData();
    
    // 기후대 이동 비용 배수
    if (ClimateDataTable && !TileData.ClimateTypeID.IsNone())
    {
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(TileData.ClimateTypeID, TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalMovementCost *= ClimateData->MovementCostMultiplier;
        }
    }
    
    // 땅 타입 이동 비용 배수 (바다는 이동 불가이므로 배수 없음)
    if (TileData.TerrainType == ETerrainType::Land && !TileData.LandTypeID.IsNone())
    {
        if (LandTypeDataTable)
        {
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(TileData.LandTypeID, TEXT("LandTypeData"));
            if (LandData)
            {
                TotalMovementCost *= LandData->MovementCostMultiplier;
            }
        }
    }
    
    return TotalMovementCost;
}

int32 UWorldComponent::CalculateBaseDefenseBonus(UWorldTile* Tile) const
{
    if (!Tile)
    {
        return 0;
    }
    
    int32 TotalDefenseBonus = 0;
    FTileData TileData = Tile->GetTileData();
    
    // 기후대 기본 방어 보너스
    if (ClimateDataTable && !TileData.ClimateTypeID.IsNone())
    {
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(TileData.ClimateTypeID, TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalDefenseBonus += ClimateData->BaseDefenseBonus;
        }
    }
    
    // 땅 타입 방어 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land && !TileData.LandTypeID.IsNone())
    {
        if (LandTypeDataTable)
        {
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(TileData.LandTypeID, TEXT("LandTypeData"));
            if (LandData)
            {
                TotalDefenseBonus += LandData->DefenseBonus;
            }
        }
    }
    
    return TotalDefenseBonus;
}

// 판게아 스타일 지형 생성 함수들
void UWorldComponent::GeneratePangaeaTerrain()
{
    // 1단계: 중심에서 큰 육지 덩어리 생성
    FVector2D CenterHex = FVector2D::ZeroVector;
    int32 LandRadius = WorldConfig.WorldRadius * 2 / 3; // 반지름의 2/3를 육지로
    
    // 모든 타일을 먼저 바다로 초기화
    for (auto& Pair : HexTiles)
    {
        Pair.Value->SetTerrainType(ETerrainType::Ocean);
    }
    
    // 중심 주변을 육지로 설정 (원형 패턴)
    for (auto& Pair : HexTiles)
    {
        FVector2D HexPos = Pair.Key;
        int32 Distance = GetHexDistance(CenterHex, HexPos);
        
        // 기본 육지 영역 설정
        float LandChance = 1.0f - (float)Distance / (float)LandRadius;
        LandChance = FMath::Clamp(LandChance, 0.0f, 1.0f);
        
        // 노이즈 추가로 자연스러운 경계 만들기
        float Noise = FMath::FRandRange(-0.3f, 0.3f);
        LandChance += Noise;
        
        if (LandChance > 0.3f)
        {
            Pair.Value->SetTerrainType(ETerrainType::Land);
        }
    }
    
    // 2단계: 연결성 보장
    EnsureLandConnectivity();
}

void UWorldComponent::EnsureLandConnectivity()
{
    // BFS로 연결된 육지 영역 찾기
    TSet<FVector2D> ConnectedLand;
    TArray<FVector2D> Queue;
    
    // 첫 번째 육지 타일을 시작점으로 찾기
    for (auto& Pair : HexTiles)
    {
        if (Pair.Value->GetTerrainType() == ETerrainType::Land)
        {
            Queue.Add(Pair.Key);
            ConnectedLand.Add(Pair.Key);
            break;
        }
    }
    
    // BFS로 연결된 모든 육지 탐색
    while (!Queue.IsEmpty())
    {
        FVector2D CurrentHex = Queue.Pop();
        
        TArray<FVector2D> Neighbors = GetHexNeighbors(CurrentHex);
        for (const FVector2D& Neighbor : Neighbors)
        {
            if (UWorldTile* NeighborTile = GetTileAtHex(Neighbor))
            {
                if (NeighborTile->GetTerrainType() == ETerrainType::Land && 
                    !ConnectedLand.Contains(Neighbor))
                {
                    ConnectedLand.Add(Neighbor);
                    Queue.Add(Neighbor);
                }
            }
        }
    }
    
    // 분리된 육지들을 메인 육지에 연결
    ConnectDisconnectedLand(ConnectedLand);
}

void UWorldComponent::ConnectDisconnectedLand(const TSet<FVector2D>& ConnectedLand)
{
    // 연결되지 않은 육지들을 찾기
    TArray<TSet<FVector2D>> DisconnectedIslands;
    
    for (auto& Pair : HexTiles)
    {
        if (Pair.Value->GetTerrainType() == ETerrainType::Land && 
            !ConnectedLand.Contains(Pair.Key))
        {
            // 새로운 섬 발견 - BFS로 섬의 모든 타일 찾기
            TSet<FVector2D> NewIsland;
            TArray<FVector2D> IslandQueue;
            IslandQueue.Add(Pair.Key);
            NewIsland.Add(Pair.Key);
            
            while (!IslandQueue.IsEmpty())
            {
                FVector2D CurrentHex = IslandQueue.Pop();
                TArray<FVector2D> Neighbors = GetHexNeighbors(CurrentHex);
                
                for (const FVector2D& Neighbor : Neighbors)
                {
                    if (UWorldTile* NeighborTile = GetTileAtHex(Neighbor))
                    {
                        if (NeighborTile->GetTerrainType() == ETerrainType::Land && 
                            !NewIsland.Contains(Neighbor))
                        {
                            NewIsland.Add(Neighbor);
                            IslandQueue.Add(Neighbor);
                        }
                    }
                }
            }
            
            DisconnectedIslands.Add(NewIsland);
        }
    }
    
    // 각 섬을 메인 육지에 연결
    for (const TSet<FVector2D>& Island : DisconnectedIslands)
    {
        ConnectIslandToMainland(Island, ConnectedLand);
    }
}

void UWorldComponent::ConnectIslandToMainland(const TSet<FVector2D>& Island, const TSet<FVector2D>& Mainland)
{
    if (Island.Num() == 0 || Mainland.Num() == 0)
    {
        return;
    }
    
    // 섬과 메인 육지의 중심점 찾기
    FVector2D IslandCenter = GetIslandCenter(Island);
    FVector2D MainlandCenter = GetIslandCenter(Mainland);
    
    // 두 중심점 사이의 직선 경로를 육지로 만들기
    int32 Distance = GetHexDistance(IslandCenter, MainlandCenter);
    
    for (int32 i = 0; i <= Distance; i++)
    {
        float T = (float)i / (float)Distance;
        FVector2D InterpolatedHex = FVector2D(
            FMath::Lerp(IslandCenter.X, MainlandCenter.X, T),
            FMath::Lerp(IslandCenter.Y, MainlandCenter.Y, T)
        );
        
        if (UWorldTile* Tile = GetTileAtHex(InterpolatedHex))
        {
            Tile->SetTerrainType(ETerrainType::Land);
        }
    }
}

FVector2D UWorldComponent::GetIslandCenter(const TSet<FVector2D>& Island) const
{
    if (Island.Num() == 0)
    {
        return FVector2D::ZeroVector;
    }
    
    FVector2D Center = FVector2D::ZeroVector;
    for (const FVector2D& Hex : Island)
    {
        Center += Hex;
    }
    
    Center /= Island.Num();
    return Center;
}
