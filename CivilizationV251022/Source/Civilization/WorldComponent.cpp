// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldComponent.h"
#include "Engine/DataTable.h"

UWorldComponent::UWorldComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    
    // 기본 월드 설정만 초기화 (임시값 제거)
    WorldConfig = FWorldConfig();
    
    bIsWorldGenerated = false;
    
    // 데이터테이블 로딩
    LoadDataTables();
}

void UWorldComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UWorldComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UWorldComponent::GenerateWorld()
{
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
    FirstSelectedTile = nullptr;
    SecondSelectedTile = nullptr;
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
        if (FirstSelectedTile == Tile)
        {
            FirstSelectedTile = nullptr;
        }
        if (SecondSelectedTile == Tile)
        {
            SecondSelectedTile = nullptr;
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
    // 원형 경계: 중심 타일(Q=0, R=0)에서의 거리가 반지름 이내인지 확인
    int32 Distance = GetHexDistance(FVector2D(0, 0), HexPosition);
    return Distance <= WorldConfig.WorldRadius;
}

void UWorldComponent::HandleTwoTileClick(UWorldTile* ClickedTile)  //나중에 반환형 바꾸는것도 고려
{
    if (!ClickedTile)
    {
        return;
    }
    
    FVector2D HexPos = ClickedTile->GetGridPosition();
    UE_LOG(LogTemp, Warning, TEXT("2단계 선택 클릭: Q=%d, R=%d"), (int32)HexPos.X, (int32)HexPos.Y);
    
    // 첫 번째 선택이 없으면 첫 번째로 설정
    if (!HasFirstSelection())
    {
        FirstSelectedTile = ClickedTile;
        FirstSelectedTile->SetSelected(true);
        
        UE_LOG(LogTemp, Warning, TEXT("첫 번째 타일 선택됨"));
        
        // 선택 이벤트 발생
        NotifyTileSelected(FirstSelectedTile);
    }
    // 첫 번째 선택이 있고 두 번째 선택이 없으면 두 번째로 설정
    else if (!HasSecondSelection())
    {
        SecondSelectedTile = ClickedTile;
        SecondSelectedTile->SetSelected(true);
        
        UE_LOG(LogTemp, Warning, TEXT("두 번째 타일 선택됨 - 액션 실행"));
        
        // 선택 이벤트 발생
        NotifyTileSelected(SecondSelectedTile);
        
        // 선택 초기화 (액션 완료 후)
        ClearSelection();
    }
}

void UWorldComponent::HandleOneTileClick(UWorldTile* ClickedTile)  //나중에 반환형 바꾸는것도 고려
{
    if (!ClickedTile)
    {
        return;
    }
    
    FVector2D HexPos = ClickedTile->GetGridPosition();
    UE_LOG(LogTemp, Warning, TEXT("1단계 선택 클릭: Q=%d, R=%d"), (int32)HexPos.X, (int32)HexPos.Y);
    
    // 기존 선택 해제
    ClearSelection();
    
    // 새로운 타일 선택
    FirstSelectedTile = ClickedTile;
    FirstSelectedTile->SetSelected(true);
    
    UE_LOG(LogTemp, Warning, TEXT("1단계 타일 선택됨 - 즉시 액션 실행"));
    
    // 선택 이벤트 발생
    NotifyTileSelected(FirstSelectedTile);
}

void UWorldComponent::ClearSelection()
{
    // 기존 선택 해제
    if (FirstSelectedTile)
    {
        FirstSelectedTile->SetSelected(false);
        FirstSelectedTile = nullptr;
    }
    
    if (SecondSelectedTile)
    {
        SecondSelectedTile->SetSelected(false);
        SecondSelectedTile = nullptr;
    }
}

void UWorldComponent::HandleTileHoverBegin(UWorldTile* HoveredTile)
{
    if (!HoveredTile)
    {
        return;
    }
    
    // 호버된 타일 정보 로그 출력 (임시)
    FVector2D HexPos = HoveredTile->GetGridPosition();
    
    // 바다 타일인지 확인
    if (HoveredTile->GetTerrainType() == ETerrainType::Ocean)
    {
        UE_LOG(LogTemp, Warning, TEXT("호버 시작: Q=%d, R=%d - 바다"), 
               (int32)HexPos.X, (int32)HexPos.Y);
    }
    else
    {
        FString ClimateName = HoveredTile->GetClimateTypeName();
        FString LandTypeName = HoveredTile->GetLandTypeName();
        
        UE_LOG(LogTemp, Warning, TEXT("호버 시작: Q=%d, R=%d - %s %s"), 
               (int32)HexPos.X, (int32)HexPos.Y, *ClimateName, *LandTypeName);
    }
}

void UWorldComponent::HandleTileHoverEnd(UWorldTile* HoveredTile)
{
    if (!HoveredTile)
    {
        return;
    }
    
    // 호버 종료 로그 출력 (임시)
    FVector2D HexPos = HoveredTile->GetGridPosition();
    UE_LOG(LogTemp, Warning, TEXT("호버 종료: Q=%d, R=%d"), (int32)HexPos.X, (int32)HexPos.Y);
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
    
    // 보너스 자원 데이터테이블 로딩
    if (!BonusResourceDataTable)
    {
        BonusResourceDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Civilization/Data/DT_BonusResourceData.DT_BonusResourceData"));
    }
    
    // 전략 자원 데이터테이블 로딩
    if (!StrategicResourceDataTable)
    {
        StrategicResourceDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Civilization/Data/DT_StrategicResourceData.DT_StrategicResourceData"));
    }
    
    // 사치 자원 데이터테이블 로딩
    if (!LuxuryResourceDataTable)
    {
        LuxuryResourceDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Game/Civilization/Data/DT_LuxuryResourceData.DT_LuxuryResourceData"));
    }
}

void UWorldComponent::GenerateTerrain()
{
    // 판게아 스타일 지형 생성 (연결된 대륙)
    GeneratePangaeaTerrain();
}

void UWorldComponent::GenerateResources()
{
    // 1단계: 육지 타일만 수집
    TArray<UWorldTile*> LandTiles;
    for (auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetTerrainType() == ETerrainType::Land)
            {
                LandTiles.Add(Tile);
            }
        }
    }
    
    if (LandTiles.Num() == 0)
    {
        return; // 육지가 없으면 종료
    }
    
    // 2단계: 사치 자원 생성
    GenerateLuxuryResources(LandTiles);
    
    // 3단계: 전략 자원 생성
    GenerateStrategicResources(LandTiles);
    
    // 4단계: 보너스 자원 생성
    GenerateBonusResources(LandTiles);
}

void UWorldComponent::GenerateLuxuryResources(TArray<UWorldTile*>& LandTiles)
{
    if (!LuxuryResourceDataTable)
    {
        return;
    }
    
    // 사치 자원 데이터 수집
    TArray<FLuxuryResourceData*> LuxuryResourceDataArray;
    LuxuryResourceDataTable->GetAllRows<FLuxuryResourceData>(TEXT("LuxuryResourceData"), LuxuryResourceDataArray);
    
    if (LuxuryResourceDataArray.Num() == 0)
    {
        return;
    }
    
    // 각 사치 자원별로 생성 시도
    for (FLuxuryResourceData* ResourceData : LuxuryResourceDataArray)
    {
        if (!ResourceData || ResourceData->LuxuryResource == ELuxuryResource::None)
        {
            continue;
        }
        
        // 호환되는 타일들 필터링
        TArray<UWorldTile*> CompatibleTiles;
        for (UWorldTile* Tile : LandTiles)
        {
            if (IsResourceCompatibleWithTile(ResourceData->CompatibleClimates, ResourceData->CompatibleLandTypes, ResourceData->bRequiresForest, Tile))
            {
                CompatibleTiles.Add(Tile);
            }
        }
        
        if (CompatibleTiles.Num() == 0)
        {
            continue; // 호환되는 타일이 없으면 건너뛰기
        }
        
        // 생성 확률에 따라 자원 배치
        int32 MaxResources = FMath::RoundToInt((float)CompatibleTiles.Num() * ResourceData->SpawnProbability);
        
        int32 ResourcesPlaced = 0;
        for (UWorldTile* Tile : CompatibleTiles)
        {
            if (ResourcesPlaced >= MaxResources)
            {
                break;
            }
            
            // 이미 자원이 있는 타일은 건너뛰기
            if (Tile->HasResource())
            {
                continue;
            }
            
            // 랜덤 확률로 자원 배치
            float RandomValue = FMath::FRand();
            if (RandomValue < ResourceData->SpawnProbability)
            {
                Tile->SetResourceCategory(EResourceCategory::Luxury);
                Tile->SetLuxuryResource(ResourceData->LuxuryResource);
                ResourcesPlaced++;
            }
        }
    }
}

void UWorldComponent::GenerateStrategicResources(TArray<UWorldTile*>& LandTiles)
{
    if (!StrategicResourceDataTable)
    {
        return;
    }
    
    // 전략 자원 데이터 수집
    TArray<FStrategicResourceData*> StrategicResourceDataArray;
    StrategicResourceDataTable->GetAllRows<FStrategicResourceData>(TEXT("StrategicResourceData"), StrategicResourceDataArray);
    
    if (StrategicResourceDataArray.Num() == 0)
    {
        return;
    }
    
    // 각 전략 자원별로 생성 시도
    for (FStrategicResourceData* ResourceData : StrategicResourceDataArray)
    {
        if (!ResourceData || ResourceData->StrategicResource == EStrategicResource::None)
        {
            continue;
        }
        
        // 호환되는 타일들 필터링
        TArray<UWorldTile*> CompatibleTiles;
        for (UWorldTile* Tile : LandTiles)
        {
            if (IsResourceCompatibleWithTile(ResourceData->CompatibleClimates, ResourceData->CompatibleLandTypes, ResourceData->bRequiresForest, Tile))
            {
                CompatibleTiles.Add(Tile);
            }
        }
        
        if (CompatibleTiles.Num() == 0)
        {
            continue; // 호환되는 타일이 없으면 건너뛰기
        }
        
        // 생성 확률에 따라 자원 배치
        int32 MaxResources = FMath::RoundToInt((float)CompatibleTiles.Num() * ResourceData->SpawnProbability);
        
        int32 ResourcesPlaced = 0;
        for (UWorldTile* Tile : CompatibleTiles)
        {
            if (ResourcesPlaced >= MaxResources)
            {
                break;
            }
            
            // 이미 자원이 있는 타일은 건너뛰기
            if (Tile->HasResource())
            {
                continue;
            }
            
            // 랜덤 확률로 자원 배치
            float RandomValue = FMath::FRand();
            if (RandomValue < ResourceData->SpawnProbability)
            {
                Tile->SetResourceCategory(EResourceCategory::Strategic);
                Tile->SetStrategicResource(ResourceData->StrategicResource);
                ResourcesPlaced++;
            }
        }
    }
}

void UWorldComponent::GenerateBonusResources(TArray<UWorldTile*>& LandTiles)
{
    if (!BonusResourceDataTable)
    {
        return;
    }
    
    // 보너스 자원 데이터 수집
    TArray<FBonusResourceData*> BonusResourceDataArray;
    BonusResourceDataTable->GetAllRows<FBonusResourceData>(TEXT("BonusResourceData"), BonusResourceDataArray);
    
    if (BonusResourceDataArray.Num() == 0)
    {
        return;
    }
    
    // 각 보너스 자원별로 생성 시도
    for (FBonusResourceData* ResourceData : BonusResourceDataArray)
    {
        if (!ResourceData || ResourceData->BonusResource == EBonusResource::None)
        {
            continue;
        }
        
        // 호환되는 타일들 필터링
        TArray<UWorldTile*> CompatibleTiles;
        for (UWorldTile* Tile : LandTiles)
        {
            if (IsResourceCompatibleWithTile(ResourceData->CompatibleClimates, ResourceData->CompatibleLandTypes, ResourceData->bRequiresForest, Tile))
            {
                CompatibleTiles.Add(Tile);
            }
        }
        
        if (CompatibleTiles.Num() == 0)
        {
            continue; // 호환되는 타일이 없으면 건너뛰기
        }
        
        // 생성 확률에 따라 자원 배치
        int32 MaxResources = FMath::RoundToInt((float)CompatibleTiles.Num() * ResourceData->SpawnProbability);
        
        int32 ResourcesPlaced = 0;
        for (UWorldTile* Tile : CompatibleTiles)
        {
            if (ResourcesPlaced >= MaxResources)
            {
                break;
            }
            
            // 이미 자원이 있는 타일은 건너뛰기
            if (Tile->HasResource())
            {
                continue;
            }
            
            // 랜덤 확률로 자원 배치
            float RandomValue = FMath::FRand();
            if (RandomValue < ResourceData->SpawnProbability)
            {
                Tile->SetResourceCategory(EResourceCategory::Bonus);
                Tile->SetBonusResource(ResourceData->BonusResource);
                ResourcesPlaced++;
            }
        }
    }
}

bool UWorldComponent::IsResourceCompatibleWithTile(const TArray<EClimateType>& CompatibleClimates, const TArray<ELandType>& CompatibleLandTypes, bool bRequiresForest, UWorldTile* Tile) const
{
    if (!Tile)
    {
        return false;
    }
    
    // 기후대 호환성 확인
    if (CompatibleClimates.Num() > 0)
    {
        // 타일의 기후대 Enum 직접 사용
        EClimateType TileClimate = Tile->GetClimateType();
        
        bool bClimateCompatible = CompatibleClimates.Contains(TileClimate);
        if (!bClimateCompatible)
        {
            return false;
        }
    }
    
    // 지형 호환성 확인
    if (CompatibleLandTypes.Num() > 0)
    {
        // 타일의 지형 Enum 직접 사용
        ELandType TileLandType = Tile->GetLandType();
        
        bool bLandTypeCompatible = CompatibleLandTypes.Contains(TileLandType);
        if (!bLandTypeCompatible)
        {
            return false;
        }
    }
    
    // 숲 조건 확인
    if (bRequiresForest && !Tile->HasForest())
    {
        return false; // 숲이 필요한데 숲이 없으면 불가능
    }
    if (!bRequiresForest && Tile->HasForest())
    {
        return false; // 숲이 없어야 하는데 숲이 있으면 불가능
    }
    
    return true;
}

void UWorldComponent::GenerateClimateZones()
{
    // 1단계: 육지 타일만 수집
    TArray<FVector2D> LandTiles;
    for (auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetTerrainType() == ETerrainType::Land)
            {
                LandTiles.Add(Pair.Key);
            }
        }
    }
    
    if (LandTiles.Num() == 0)
    {
        return; // 육지가 없으면 종료
    }
    
    // 2단계: 북쪽/남쪽 중앙점 계산 (수학적 좌표)
    FVector2D NorthCenter = FVector2D(-WorldConfig.WorldRadius / 2.0f, WorldConfig.WorldRadius);
    FVector2D SouthCenter = FVector2D(WorldConfig.WorldRadius / 2.0f, -WorldConfig.WorldRadius);
    
    // 3단계: 툰드라 가중치 계산 및 정렬 (북쪽 중앙에 가까울수록 높음)
    TArray<TPair<FVector2D, float>> TundraWeights;
    float MaxDistance = (float)WorldConfig.WorldRadius * 2; // 최대 거리 (정규화용)
    
    for (const FVector2D& HexPos : LandTiles)
    {
        int32 DistanceToNorth = GetHexDistance(HexPos, NorthCenter);
        float Noise = FMath::FRandRange(-0.2f, 0.2f); // 노이즈
        float TundraWeight = MaxDistance - (float)DistanceToNorth + Noise;
        
        TundraWeights.Add(TPair<FVector2D, float>(HexPos, TundraWeight));
    }
    
    // 툰드라 가중치 내림차순 정렬
    TundraWeights.Sort([](const TPair<FVector2D, float>& A, const TPair<FVector2D, float>& B)
    {
        return A.Value > B.Value;
    });
    
    // 4단계: 사막 가중치 계산 및 정렬 (남쪽 중앙에 가까울수록 높음)
    TArray<TPair<FVector2D, float>> DesertWeights;
    
    for (const FVector2D& HexPos : LandTiles)
    {
        int32 DistanceToSouth = GetHexDistance(HexPos, SouthCenter);
        float Noise = FMath::FRandRange(-0.2f, 0.2f); // 노이즈
        float DesertWeight = MaxDistance - (float)DistanceToSouth + Noise;
        
        DesertWeights.Add(TPair<FVector2D, float>(HexPos, DesertWeight));
    }
    
    // 사막 가중치 내림차순 정렬
    DesertWeights.Sort([](const TPair<FVector2D, float>& A, const TPair<FVector2D, float>& B)
    {
        return A.Value > B.Value;
    });
    
    // 5단계: 모든 육지를 먼저 온대로 초기화
    for (const FVector2D& HexPos : LandTiles)
    {
        if (UWorldTile* Tile = GetTileAtHex(HexPos))
        {
            Tile->SetClimateType(EClimateType::Temperate);
        }
    }
    
    // 6단계: 툰드라 할당 (북쪽 중앙에 가까운 순서대로)
    int32 TundraCount = FMath::RoundToInt((float)LandTiles.Num() * WorldConfig.TundraPercentage);
    TSet<FVector2D> AssignedTiles; // 중복 방지용
    
    for (int32 i = 0; i < TundraCount && i < TundraWeights.Num(); i++)
    {
        FVector2D HexPos = TundraWeights[i].Key;
        if (UWorldTile* Tile = GetTileAtHex(HexPos))
        {
            Tile->SetClimateType(EClimateType::Tundra);
            AssignedTiles.Add(HexPos);
        }
    }
    
    // 7단계: 사막 할당 (남쪽 중앙에 가까운 순서대로, 이미 툰드라인 타일 제외)
    int32 DesertCount = FMath::RoundToInt((float)LandTiles.Num() * WorldConfig.DesertPercentage);
    
    for (int32 i = 0; i < DesertWeights.Num() && AssignedTiles.Num() < TundraCount + DesertCount; i++)
    {
        FVector2D HexPos = DesertWeights[i].Key;
        
        // 이미 툰드라로 할당된 타일은 건너뛰기
        if (AssignedTiles.Contains(HexPos))
        {
            continue;
        }
        
        if (UWorldTile* Tile = GetTileAtHex(HexPos))
        {
            Tile->SetClimateType(EClimateType::Desert);
            AssignedTiles.Add(HexPos);
        }
    }
}

void UWorldComponent::GenerateLandTypes()
{
    // 기본 땅 타입 생성
    for (auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetTerrainType() == ETerrainType::Land)
            {
                float RandomValue = FMath::FRand();
                
                if (RandomValue < WorldConfig.PlainsPercentage)
                {
                    Tile->SetLandType(ELandType::Plains);
                }
                else if (RandomValue < WorldConfig.PlainsPercentage + WorldConfig.HillsPercentage)
                {
                    Tile->SetLandType(ELandType::Hills);
                }
                else
                {
                    Tile->SetLandType(ELandType::Mountains);
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
    int32 Count = 0;
    for (const auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetClimateType() == ClimateType)
            {
                Count++;
            }
        }
    }
    return Count;
}

int32 UWorldComponent::GetLandTypeTileCount(ELandType LandType) const
{
    int32 Count = 0;
    for (const auto& Pair : HexTiles)
    {
        if (UWorldTile* Tile = Pair.Value)
        {
            if (Tile->GetLandType() == LandType)
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
    TArray<FVector2D> Path;

    // 시작점과 끝점이 같으면 시작점만 반환
    if (StartHex == EndHex)
    {
        Path.Add(StartHex);
        return Path;
    }

    // 유효하지 않은 좌표 확인
    if (!IsValidHexPosition(StartHex) || !IsValidHexPosition(EndHex))
    {
        return Path; // 빈 경로 반환
    }

    // 목표점에 도달할 수 없는지 확인
    if (!CanMoveToHex(EndHex))
    {
        return Path; // 빈 경로 반환
    }

    // A* 알고리즘 구현
    TMap<FVector2D, FAStarNode> OpenSet; // 오픈 리스트 (우선순위 큐 대신 맵 사용)
    TMap<FVector2D, FAStarNode> ClosedSet; // 클로즈드 리스트
    TMap<FVector2D, FAStarNode> CameFrom; // 경로 추적용

    // 시작 노드 초기화
    int32 StartHeuristic = CalculateHeuristic(StartHex, EndHex);
    FAStarNode StartNode(StartHex, 0, StartHeuristic, FVector2D::ZeroVector, true);
    OpenSet.Add(StartHex, StartNode);

    int32 IterationCount = 0;
    while (OpenSet.Num() > 0)
    {
        // 오픈 리스트에서 F값이 가장 작은 노드 찾기
        FVector2D CurrentHex = FVector2D::ZeroVector;
        int32 LowestFCost = INT32_MAX;

        for (const auto& Pair : OpenSet)
        {
            if (Pair.Value.FCost < LowestFCost)
            {
                LowestFCost = Pair.Value.FCost;
                CurrentHex = Pair.Key;
            }
        }

        // 현재 노드를 클로즈드 리스트로 이동
        FAStarNode CurrentNode = OpenSet[CurrentHex];
        OpenSet.Remove(CurrentHex);
        ClosedSet.Add(CurrentHex, CurrentNode);

        // 목표에 도달했는지 확인
        if (CurrentHex == EndHex)
        {
            Path = ReconstructPath(CameFrom, CurrentHex);
            break;
        }

        // 인접한 육각형들 확인
        TArray<FVector2D> Neighbors = GetHexNeighbors(CurrentHex);

        for (const FVector2D& NeighborHex : Neighbors)
        {
            // 유효하지 않은 위치이거나 층수 이동이 불가능한 곳은 건너뛰기
            if (!IsValidHexPosition(NeighborHex) || !CanMoveBetweenHexes(CurrentHex, NeighborHex))
            {
                continue;
            }

            // 이미 클로즈드 리스트에 있으면 건너뛰기
            if (ClosedSet.Contains(NeighborHex))
            {
                continue;
            }

            // 현재 노드에서 이웃까지의 이동 비용 계산
            int32 MovementCost = GetMovementCostBetweenHexes(CurrentHex, NeighborHex);
            int32 TentativeGCost = CurrentNode.GCost + MovementCost;

            // 이웃이 오픈 리스트에 있는지 확인
            bool bInOpenSet = OpenSet.Contains(NeighborHex);

            // 더 나은 경로를 찾았거나 이웃이 오픈 리스트에 없으면
            if (!bInOpenSet || TentativeGCost < OpenSet[NeighborHex].GCost)
            {
                // 이웃 노드 생성
                int32 Heuristic = CalculateHeuristic(NeighborHex, EndHex);
                FAStarNode NeighborNode(NeighborHex, TentativeGCost, Heuristic, CurrentHex, true);

                // 오픈 리스트에 추가 또는 업데이트
                OpenSet.Add(NeighborHex, NeighborNode);
                CameFrom.Add(NeighborHex, NeighborNode);
            }
        }

        IterationCount++;

        // 너무 많은 반복을 방지 (안전장치)
        if (IterationCount > 10000)
        {
            break;
        }
    }

    return Path;
}

TArray<FVector2D> UWorldComponent::FindPathWithMovementCost(FVector2D StartHex, FVector2D EndHex, int32 MaxMovementCost) const
{
    // A* 알고리즘으로 경로를 찾은 후, 최대 이동 비용을 초과하는지 확인
    TArray<FVector2D> Path = FindPath(StartHex, EndHex);

    if (Path.Num() <= 1)
    {
        return Path; // 경로가 없거나 시작점만 있는 경우
    }

    // 경로의 총 이동 비용 계산
    int32 TotalMovementCost = 0;
    for (int32 i = 0; i < Path.Num() - 1; i++)
    {
        TotalMovementCost += GetMovementCostBetweenHexes(Path[i], Path[i + 1]);

        // 최대 이동 비용을 초과하면 여기서 경로를 잘라냄
        if (TotalMovementCost > MaxMovementCost)
        {
            Path.SetNum(i + 1); // 현재까지의 경로만 유지
            break;
        }
    }

    return Path;
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

int32 UWorldComponent::CalculateHeuristic(FVector2D StartHex, FVector2D EndHex) const
{
    // 육각형 거리를 휴리스틱으로 사용 (일반적으로 1의 이동 비용을 가정)
    int32 HexDistance = GetHexDistance(StartHex, EndHex);
    return HexDistance;
}

TArray<FVector2D> UWorldComponent::ReconstructPath(const TMap<FVector2D, FAStarNode>& CameFrom, FVector2D Current) const
{
    TArray<FVector2D> Path;

    // 시작점부터 목표점까지의 경로 재구성
    TArray<FVector2D> ReversedPath;
    ReversedPath.Add(Current);

    // 목표점에서 시작점까지 역순으로 경로 수집
    while (CameFrom.Contains(Current))
    {
        const FAStarNode& CurrentNode = CameFrom[Current];
        Current = CurrentNode.ParentHex;

        // 시작점에 도달했는지 확인 (부모가 (0,0)인 경우)
        if (Current == FVector2D::ZeroVector)
        {
            ReversedPath.Add(FVector2D(0, 0)); // 시작점 추가
            break;
        }

        ReversedPath.Add(Current);
    }

    // 역순으로 수집된 경로를 올바른 순서로 뒤집기
    for (int32 i = ReversedPath.Num() - 1; i >= 0; i--)
    {
        Path.Add(ReversedPath[i]);
    }

    return Path;
}

int32 UWorldComponent::GetMovementCostBetweenHexes(FVector2D FromHex, FVector2D ToHex) const
{
    // 층수 이동 가능성 먼저 체크
    if (!CanMoveBetweenHexes(FromHex, ToHex))
    {
        return INT32_MAX;
    }
    
    // 층수를 고려한 이동비용 계산
    return GetMovementCostBetweenHexesWithFloor(FromHex, ToHex);
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
    if (ClimateDataTable)
    {
        // Enum을 문자열로 변환해서 데이터테이블에서 찾기
        FString ClimateString = UEnum::GetValueAsString(TileData.ClimateType);
        FString ClimateName = ClimateString.RightChop(ClimateString.Find(TEXT("::")) + 2);
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(FName(*ClimateName), TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalFood += ClimateData->BaseFoodYield;
        }
    }
    
    // 땅 타입 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land)
    {
        if (LandTypeDataTable)
        {
            // Enum을 문자열로 변환해서 데이터테이블에서 찾기
            FString LandString = UEnum::GetValueAsString(TileData.LandType);
            FString LandName = LandString.RightChop(LandString.Find(TEXT("::")) + 2);
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(FName(*LandName), TEXT("LandTypeData"));
            if (LandData)
            {
                TotalFood += LandData->FoodBonus;
            }
        }
    }
    
    // 자원 보너스 추가 (개선시설 없이도 적용)
    if (TileData.TerrainType == ETerrainType::Land && TileData.ResourceCategory != EResourceCategory::None)
    {
        switch (TileData.ResourceCategory)
        {
        case EResourceCategory::Bonus:
            if (BonusResourceDataTable && TileData.BonusResource != EBonusResource::None)
            {
                FBonusResourceData* ResourceData = BonusResourceDataTable->FindRow<FBonusResourceData>(FName(*UEnum::GetValueAsString(TileData.BonusResource).RightChop(UEnum::GetValueAsString(TileData.BonusResource).Find(TEXT("::")) + 2)), TEXT("BonusResourceData"));
                if (ResourceData)
                {
                    TotalFood += ResourceData->FoodYield;
                }
            }
            break;
            
        case EResourceCategory::Strategic:
            if (StrategicResourceDataTable && TileData.StrategicResource != EStrategicResource::None)
            {
                FStrategicResourceData* ResourceData = StrategicResourceDataTable->FindRow<FStrategicResourceData>(FName(*UEnum::GetValueAsString(TileData.StrategicResource).RightChop(UEnum::GetValueAsString(TileData.StrategicResource).Find(TEXT("::")) + 2)), TEXT("StrategicResourceData"));
                if (ResourceData)
                {
                    TotalFood += ResourceData->FoodYield;
                }
            }
            break;
            
        case EResourceCategory::Luxury:
            if (LuxuryResourceDataTable && TileData.LuxuryResource != ELuxuryResource::None)
            {
                FLuxuryResourceData* ResourceData = LuxuryResourceDataTable->FindRow<FLuxuryResourceData>(FName(*UEnum::GetValueAsString(TileData.LuxuryResource).RightChop(UEnum::GetValueAsString(TileData.LuxuryResource).Find(TEXT("::")) + 2)), TEXT("LuxuryResourceData"));
                if (ResourceData)
                {
                    TotalFood += ResourceData->FoodYield;
                }
            }
            break;
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
    if (ClimateDataTable)
    {
        // Enum을 문자열로 변환해서 데이터테이블에서 찾기
        FString ClimateString = UEnum::GetValueAsString(TileData.ClimateType);
        FString ClimateName = ClimateString.RightChop(ClimateString.Find(TEXT("::")) + 2);
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(FName(*ClimateName), TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalProduction += ClimateData->BaseProductionYield;
        }
    }
    
    // 땅 타입 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land)
    {
        if (LandTypeDataTable)
        {
            // Enum을 문자열로 변환해서 데이터테이블에서 찾기
            FString LandString = UEnum::GetValueAsString(TileData.LandType);
            FString LandName = LandString.RightChop(LandString.Find(TEXT("::")) + 2);
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(FName(*LandName), TEXT("LandTypeData"));
            if (LandData)
            {
                TotalProduction += LandData->ProductionBonus;
            }
        }
    }
    
    // 자원 보너스 추가 (개선시설 없이도 적용)
    if (TileData.TerrainType == ETerrainType::Land && TileData.ResourceCategory != EResourceCategory::None)
    {
        switch (TileData.ResourceCategory)
        {
        case EResourceCategory::Bonus:
            if (BonusResourceDataTable && TileData.BonusResource != EBonusResource::None)
            {
                FBonusResourceData* ResourceData = BonusResourceDataTable->FindRow<FBonusResourceData>(FName(*UEnum::GetValueAsString(TileData.BonusResource).RightChop(UEnum::GetValueAsString(TileData.BonusResource).Find(TEXT("::")) + 2)), TEXT("BonusResourceData"));
                if (ResourceData)
                {
                    TotalProduction += ResourceData->ProductionYield;
                }
            }
            break;
            
        case EResourceCategory::Strategic:
            if (StrategicResourceDataTable && TileData.StrategicResource != EStrategicResource::None)
            {
                FStrategicResourceData* ResourceData = StrategicResourceDataTable->FindRow<FStrategicResourceData>(FName(*UEnum::GetValueAsString(TileData.StrategicResource).RightChop(UEnum::GetValueAsString(TileData.StrategicResource).Find(TEXT("::")) + 2)), TEXT("StrategicResourceData"));
                if (ResourceData)
                {
                    TotalProduction += ResourceData->ProductionYield;
                }
            }
            break;
            
        case EResourceCategory::Luxury:
            if (LuxuryResourceDataTable && TileData.LuxuryResource != ELuxuryResource::None)
            {
                FLuxuryResourceData* ResourceData = LuxuryResourceDataTable->FindRow<FLuxuryResourceData>(FName(*UEnum::GetValueAsString(TileData.LuxuryResource).RightChop(UEnum::GetValueAsString(TileData.LuxuryResource).Find(TEXT("::")) + 2)), TEXT("LuxuryResourceData"));
                if (ResourceData)
                {
                    TotalProduction += ResourceData->ProductionYield;
                }
            }
            break;
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
    if (ClimateDataTable)
    {
        // Enum을 문자열로 변환해서 데이터테이블에서 찾기
        FString ClimateString = UEnum::GetValueAsString(TileData.ClimateType);
        FString ClimateName = ClimateString.RightChop(ClimateString.Find(TEXT("::")) + 2);
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(FName(*ClimateName), TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalGold += ClimateData->BaseGoldYield;
        }
    }
    
    // 땅 타입 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land)
    {
        if (LandTypeDataTable)
        {
            // Enum을 문자열로 변환해서 데이터테이블에서 찾기
            FString LandString = UEnum::GetValueAsString(TileData.LandType);
            FString LandName = LandString.RightChop(LandString.Find(TEXT("::")) + 2);
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(FName(*LandName), TEXT("LandTypeData"));
            if (LandData)
            {
                TotalGold += LandData->GoldBonus;
            }
        }
    }
    
    // 자원 보너스 추가 (개선시설 없이도 적용)
    if (TileData.TerrainType == ETerrainType::Land && TileData.ResourceCategory != EResourceCategory::None)
    {
        switch (TileData.ResourceCategory)
        {
        case EResourceCategory::Bonus:
            if (BonusResourceDataTable && TileData.BonusResource != EBonusResource::None)
            {
                FBonusResourceData* ResourceData = BonusResourceDataTable->FindRow<FBonusResourceData>(FName(*UEnum::GetValueAsString(TileData.BonusResource).RightChop(UEnum::GetValueAsString(TileData.BonusResource).Find(TEXT("::")) + 2)), TEXT("BonusResourceData"));
                if (ResourceData)
                {
                    TotalGold += ResourceData->GoldYield;
                }
            }
            break;
            
        case EResourceCategory::Strategic:
            if (StrategicResourceDataTable && TileData.StrategicResource != EStrategicResource::None)
            {
                FStrategicResourceData* ResourceData = StrategicResourceDataTable->FindRow<FStrategicResourceData>(FName(*UEnum::GetValueAsString(TileData.StrategicResource).RightChop(UEnum::GetValueAsString(TileData.StrategicResource).Find(TEXT("::")) + 2)), TEXT("StrategicResourceData"));
                if (ResourceData)
                {
                    TotalGold += ResourceData->GoldYield;
                }
            }
            break;
            
        case EResourceCategory::Luxury:
            if (LuxuryResourceDataTable && TileData.LuxuryResource != ELuxuryResource::None)
            {
                FLuxuryResourceData* ResourceData = LuxuryResourceDataTable->FindRow<FLuxuryResourceData>(FName(*UEnum::GetValueAsString(TileData.LuxuryResource).RightChop(UEnum::GetValueAsString(TileData.LuxuryResource).Find(TEXT("::")) + 2)), TEXT("LuxuryResourceData"));
                if (ResourceData)
                {
                    TotalGold += ResourceData->GoldYield;
                }
            }
            break;
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
    if (ClimateDataTable)
    {
        // Enum을 문자열로 변환해서 데이터테이블에서 찾기
        FString ClimateString = UEnum::GetValueAsString(TileData.ClimateType);
        FString ClimateName = ClimateString.RightChop(ClimateString.Find(TEXT("::")) + 2);
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(FName(*ClimateName), TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalScience += ClimateData->BaseScienceYield;
        }
    }
    
    // 땅 타입 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land)
    {
        if (LandTypeDataTable)
        {
            // Enum을 문자열로 변환해서 데이터테이블에서 찾기
            FString LandString = UEnum::GetValueAsString(TileData.LandType);
            FString LandName = LandString.RightChop(LandString.Find(TEXT("::")) + 2);
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(FName(*LandName), TEXT("LandTypeData"));
            if (LandData)
            {
                TotalScience += LandData->ScienceBonus;
            }
        }
    }
    
    // 자원 보너스 추가 (개선시설 없이도 적용)
    if (TileData.TerrainType == ETerrainType::Land && TileData.ResourceCategory != EResourceCategory::None)
    {
        switch (TileData.ResourceCategory)
        {
        case EResourceCategory::Bonus:
            if (BonusResourceDataTable && TileData.BonusResource != EBonusResource::None)
            {
                FBonusResourceData* ResourceData = BonusResourceDataTable->FindRow<FBonusResourceData>(FName(*UEnum::GetValueAsString(TileData.BonusResource).RightChop(UEnum::GetValueAsString(TileData.BonusResource).Find(TEXT("::")) + 2)), TEXT("BonusResourceData"));
                if (ResourceData)
                {
                    TotalScience += ResourceData->ScienceYield;
                }
            }
            break;
            
        case EResourceCategory::Strategic:
            if (StrategicResourceDataTable && TileData.StrategicResource != EStrategicResource::None)
            {
                FStrategicResourceData* ResourceData = StrategicResourceDataTable->FindRow<FStrategicResourceData>(FName(*UEnum::GetValueAsString(TileData.StrategicResource).RightChop(UEnum::GetValueAsString(TileData.StrategicResource).Find(TEXT("::")) + 2)), TEXT("StrategicResourceData"));
                if (ResourceData)
                {
                    TotalScience += ResourceData->ScienceYield;
                }
            }
            break;
            
        case EResourceCategory::Luxury:
            if (LuxuryResourceDataTable && TileData.LuxuryResource != ELuxuryResource::None)
            {
                FLuxuryResourceData* ResourceData = LuxuryResourceDataTable->FindRow<FLuxuryResourceData>(FName(*UEnum::GetValueAsString(TileData.LuxuryResource).RightChop(UEnum::GetValueAsString(TileData.LuxuryResource).Find(TEXT("::")) + 2)), TEXT("LuxuryResourceData"));
                if (ResourceData)
                {
                    TotalScience += ResourceData->ScienceYield;
                }
            }
            break;
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
    if (ClimateDataTable)
    {
        // Enum을 문자열로 변환해서 데이터테이블에서 찾기
        FString ClimateString = UEnum::GetValueAsString(TileData.ClimateType);
        FString ClimateName = ClimateString.RightChop(ClimateString.Find(TEXT("::")) + 2);
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(FName(*ClimateName), TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalFaith += ClimateData->BaseFaithYield;
        }
    }
    
    // 땅 타입 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land)
    {
        if (LandTypeDataTable)
        {
            // Enum을 문자열로 변환해서 데이터테이블에서 찾기
            FString LandString = UEnum::GetValueAsString(TileData.LandType);
            FString LandName = LandString.RightChop(LandString.Find(TEXT("::")) + 2);
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(FName(*LandName), TEXT("LandTypeData"));
            if (LandData)
            {
                TotalFaith += LandData->FaithBonus;
            }
        }
    }
    
    // 자원 보너스 추가 (개선시설 없이도 적용)
    if (TileData.TerrainType == ETerrainType::Land && TileData.ResourceCategory != EResourceCategory::None)
    {
        switch (TileData.ResourceCategory)
        {
        case EResourceCategory::Bonus:
            if (BonusResourceDataTable && TileData.BonusResource != EBonusResource::None)
            {
                FBonusResourceData* ResourceData = BonusResourceDataTable->FindRow<FBonusResourceData>(FName(*UEnum::GetValueAsString(TileData.BonusResource).RightChop(UEnum::GetValueAsString(TileData.BonusResource).Find(TEXT("::")) + 2)), TEXT("BonusResourceData"));
                if (ResourceData)
                {
                    TotalFaith += ResourceData->FaithYield;
                }
            }
            break;
            
        case EResourceCategory::Strategic:
            if (StrategicResourceDataTable && TileData.StrategicResource != EStrategicResource::None)
            {
                FStrategicResourceData* ResourceData = StrategicResourceDataTable->FindRow<FStrategicResourceData>(FName(*UEnum::GetValueAsString(TileData.StrategicResource).RightChop(UEnum::GetValueAsString(TileData.StrategicResource).Find(TEXT("::")) + 2)), TEXT("StrategicResourceData"));
                if (ResourceData)
                {
                    TotalFaith += ResourceData->FaithYield;
                }
            }
            break;
            
        case EResourceCategory::Luxury:
            if (LuxuryResourceDataTable && TileData.LuxuryResource != ELuxuryResource::None)
            {
                FLuxuryResourceData* ResourceData = LuxuryResourceDataTable->FindRow<FLuxuryResourceData>(FName(*UEnum::GetValueAsString(TileData.LuxuryResource).RightChop(UEnum::GetValueAsString(TileData.LuxuryResource).Find(TEXT("::")) + 2)), TEXT("LuxuryResourceData"));
                if (ResourceData)
                {
                    TotalFaith += ResourceData->FaithYield;
                }
            }
            break;
        }
    }
    
    return TotalFaith;
}

int32 UWorldComponent::CalculateBaseMovementCost(UWorldTile* Tile) const
{
    if (!Tile)
    {
        return 1;
    }
    
    int32 TotalMovementCost = 1;
    FTileData TileData = Tile->GetTileData();
    
    // 기후대 이동 비용 증가량
    if (ClimateDataTable)
    {
        // Enum을 문자열로 변환해서 데이터테이블에서 찾기
        FString ClimateString = UEnum::GetValueAsString(TileData.ClimateType);
        FString ClimateName = ClimateString.RightChop(ClimateString.Find(TEXT("::")) + 2);
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(FName(*ClimateName), TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalMovementCost += ClimateData->MovementCost;
        }
    }
    
    // 땅 타입 이동 비용 증가량 (바다는 이동 불가이므로 증가량 없음)
    if (TileData.TerrainType == ETerrainType::Land)
    {
        if (LandTypeDataTable)
        {
            // Enum을 문자열로 변환해서 데이터테이블에서 찾기
            FString LandString = UEnum::GetValueAsString(TileData.LandType);
            FString LandName = LandString.RightChop(LandString.Find(TEXT("::")) + 2);
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(FName(*LandName), TEXT("LandTypeData"));
            if (LandData)
            {
                TotalMovementCost += LandData->MovementCost;
            }
        }
    }
    
    // 숲이 있으면 이동 비용 +1
    if (TileData.bHasForest)
    {
        TotalMovementCost += 1;
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
    if (ClimateDataTable)
    {
        // Enum을 문자열로 변환해서 데이터테이블에서 찾기
        FString ClimateString = UEnum::GetValueAsString(TileData.ClimateType);
        FString ClimateName = ClimateString.RightChop(ClimateString.Find(TEXT("::")) + 2);
        FClimateData* ClimateData = ClimateDataTable->FindRow<FClimateData>(FName(*ClimateName), TEXT("ClimateData"));
        if (ClimateData)
        {
            TotalDefenseBonus += ClimateData->BaseDefenseBonus;
        }
    }
    
    // 땅 타입 방어 보너스 (바다는 이동 불가이므로 보너스 없음)
    if (TileData.TerrainType == ETerrainType::Land)
    {
        if (LandTypeDataTable)
        {
            // Enum을 문자열로 변환해서 데이터테이블에서 찾기
            FString LandString = UEnum::GetValueAsString(TileData.LandType);
            FString LandName = LandString.RightChop(LandString.Find(TEXT("::")) + 2);
            FLandTypeData* LandData = LandTypeDataTable->FindRow<FLandTypeData>(FName(*LandName), TEXT("LandTypeData"));
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
    // 모든 타일을 먼저 바다로 초기화
    for (auto& Pair : HexTiles)
    {
        Pair.Value->SetTerrainType(ETerrainType::Ocean);
    }
    
    // 1단계: 각 타일의 "육지 가중치" 계산 (거리 + 노이즈)
    TArray<TPair<FVector2D, float>> TileWeights; // <타일 좌표, 가중치>
    FVector2D CenterHex = FVector2D(0, 0); // 중심 타일 Q=0, R=0
    float MaxRadius = (float)WorldConfig.WorldRadius;
    
    for (auto& Pair : HexTiles)
    {
        FVector2D HexPos = Pair.Key;
        int32 Distance = GetHexDistance(CenterHex, HexPos);
        
        // 중심에서 가까울수록 높은 가중치
        float DistanceWeight = 1.0f - ((float)Distance / MaxRadius);
        
        // 노이즈 추가로 자연스러운 경계 만들기 (-0.2 ~ 0.2)
        float Noise = FMath::FRandRange(-0.2f, 0.2f);
        
        // 최종 가중치 = 거리 가중치 + 노이즈
        float FinalWeight = DistanceWeight + Noise;
        
        TileWeights.Add(TPair<FVector2D, float>(HexPos, FinalWeight));
    }
    
    // 2단계: 가중치 순으로 내림차순 정렬 (높은 가중치 = 육지 우선)
    TileWeights.Sort([](const TPair<FVector2D, float>& A, const TPair<FVector2D, float>& B)
    {
        return A.Value > B.Value; // 내림차순
    });
    
    // 3단계: 정확한 육지 비율만큼 상위 타일을 육지로 설정
    int32 TotalTiles = HexTiles.Num();
    int32 LandTileCount = FMath::RoundToInt((float)TotalTiles * (1.0f - WorldConfig.OceanPercentage));
    
    for (int32 i = 0; i < LandTileCount && i < TileWeights.Num(); i++)
    {
        FVector2D HexPos = TileWeights[i].Key;
        if (UWorldTile* Tile = GetTileAtHex(HexPos))
        {
            Tile->SetTerrainType(ETerrainType::Land);
        }
    }
}

// 층수 시스템 관련 함수들 구현
int32 UWorldComponent::GetFloorLevel(ELandType LandType) const
{
    switch (LandType)
    {
    case ELandType::Plains:
        return 1; // 1층
    case ELandType::Hills:
        return 2; // 2층
    case ELandType::Mountains:
        return 3; // 3층
    default:
        return 1; // 기본값은 1층
    }
}

bool UWorldComponent::CanMoveBetweenHexes(FVector2D FromHex, FVector2D ToHex) const
{
    UWorldTile* FromTile = GetTileAtHex(FromHex);
    UWorldTile* ToTile = GetTileAtHex(ToHex);
    
    if (!FromTile || !ToTile)
    {
        return false; // 타일이 없으면 이동 불가
    }
    
    // 바다로는 이동 불가
    if (ToTile->GetTerrainType() == ETerrainType::Ocean)
    {
        return false;
    }
    
    // 바다에서 출발도 불가
    if (FromTile->GetTerrainType() == ETerrainType::Ocean)
    {
        return false;
    }
    
    // 층수 차이 계산
    int32 FromFloor = GetFloorLevel(FromTile->GetLandType());
    int32 ToFloor = GetFloorLevel(ToTile->GetLandType());
    int32 FloorDifference = FMath::Abs(ToFloor - FromFloor);
    
    // 2층 이상 차이나면 이동 불가
    if (FloorDifference >= 2)
    {
        return false;
    }
    
    return true;
}

int32 UWorldComponent::GetMovementCostBetweenHexesWithFloor(FVector2D FromHex, FVector2D ToHex) const
{
    UWorldTile* FromTile = GetTileAtHex(FromHex);
    UWorldTile* ToTile = GetTileAtHex(ToHex);
    
    if (!FromTile || !ToTile)
    {
        return INT32_MAX; // 타일이 없으면 이동 불가
    }
    
    // 기본 이동 비용 (도착 타일의 총 이동 비용)
    int32 BaseMovementCost = ToTile->GetTotalMovementCost();
    
    // 층수 차이 계산
    int32 FromFloor = GetFloorLevel(FromTile->GetLandType());
    int32 ToFloor = GetFloorLevel(ToTile->GetLandType());
    int32 FloorDifference = ToFloor - FromFloor;
    
    int32 TotalMovementCost = BaseMovementCost;
    
    if (FloorDifference > 0)
    {
        // 올라가는 경우: 기본비용 + 층수차이
        TotalMovementCost += FloorDifference;
    }
    else if (FloorDifference < 0)
    {
        // 내려가는 경우: 기본비용만 (추가 비용 없음)
        TotalMovementCost = BaseMovementCost;
    }
    else
    {
        // 같은 층: 기본비용만
        TotalMovementCost = BaseMovementCost;
    }
    
    return TotalMovementCost;
}
