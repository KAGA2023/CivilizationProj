// Fill out your copyright notice in the Description page of Project Settings.

#include "SuperPlayerState.h"
#include "WorldComponent.h"
#include "Engine/World.h"

ASuperPlayerState::ASuperPlayerState()
{
    // 자원 초기화
    FoodGauge = FResourceGauge(); // 기본값으로 초기화
    Production = 0;
    Gold = 0;
    Science = 0;
    Faith = 0;

    // 게임 진행 초기화
    Population = 1;
    LimitPopulation = 4;

    // 통계 초기화
    TotalFoodProduced = 0;
    TotalProductionProduced = 0;
    TotalGoldEarned = 0;
    TotalScienceGained = 0;
    TotalFaithGained = 0;

    // 타일 관리 초기화
    TotalOwnedTiles = 0;
    OwnedTileCoordinates.Empty();

    // 사치 자원 관리 초기화
    OwnedLuxuryResources.Empty();
}

void ASuperPlayerState::BeginPlay()
{
    Super::BeginPlay();
    
    // 플레이어 초기화
    InitializePlayer();
}

// ========== 자원 관리 함수들 ==========
void ASuperPlayerState::AddFood(int32 Amount)
{
    if (Amount > 0)
    {
        bool bCanIncreasePopulation = FoodGauge.AddAmount(Amount);
        TotalFoodProduced += Amount;
        
        // 게이지가 가득 차면 인구 증가
        if (bCanIncreasePopulation && Population < LimitPopulation)
        {
            Population++;
        }
    }
}

void ASuperPlayerState::AddProduction(int32 Amount)
{
    if (Amount > 0)
    {
        Production += Amount;
        TotalProductionProduced += Amount;
    }
}

void ASuperPlayerState::AddGold(int32 Amount)
{
    if (Amount > 0)
    {
        Gold += Amount;
        TotalGoldEarned += Amount;
    }
}

void ASuperPlayerState::AddScience(int32 Amount)
{
    if (Amount > 0)
    {
        Science += Amount;
        TotalScienceGained += Amount;
    }
}

void ASuperPlayerState::AddFaith(int32 Amount)
{
    if (Amount > 0)
    {
        Faith += Amount;
        TotalFaithGained += Amount;
    }
}


// ========== 게이지 관리 함수들 ==========
void ASuperPlayerState::SetFoodGaugeMax(int32 MaxAmount)
{
    if (MaxAmount > 0)
    {
        FoodGauge.MaxAmount = MaxAmount;
    }
}

bool ASuperPlayerState::SpendProduction(int32 Amount)
{
    if (Amount <= 0 || Production < Amount)
    {
        return false;
    }
    
    Production -= Amount;
    return true;
}

bool ASuperPlayerState::SpendGold(int32 Amount)
{
    if (Amount <= 0 || Gold < Amount)
    {
        return false;
    }
    
    Gold -= Amount;
    return true;
}

bool ASuperPlayerState::SpendScience(int32 Amount)
{
    if (Amount <= 0 || Science < Amount)
    {
        return false;
    }
    
    Science -= Amount;
    return true;
}

bool ASuperPlayerState::SpendFaith(int32 Amount)
{
    if (Amount <= 0 || Faith < Amount)
    {
        return false;
    }
    
    Faith -= Amount;
    return true;
}

// ========== 타일 관리 함수들 ==========
void ASuperPlayerState::AddOwnedTile(FVector2D TileCoordinate, UWorldComponent* WorldComponent)
{
    if (!OwnedTileCoordinates.Contains(TileCoordinate))
    {
        OwnedTileCoordinates.Add(TileCoordinate);
        TotalOwnedTiles = OwnedTileCoordinates.Num();
        
        // WorldComponent를 통해 해당 타일의 소유 상태 업데이트
        if (WorldComponent)
        {
            if (UWorldTile* Tile = WorldComponent->GetTileAtHex(TileCoordinate))
            {
                Tile->SetOwned(true);
                Tile->SetOwnerPlayerID(GetPlayerId());
            }
        }
    }
}


void ASuperPlayerState::RemoveOwnedTile(FVector2D TileCoordinate, UWorldComponent* WorldComponent)
{
    if (OwnedTileCoordinates.Contains(TileCoordinate))
    {
        OwnedTileCoordinates.Remove(TileCoordinate);
        TotalOwnedTiles = OwnedTileCoordinates.Num();
        
        // WorldComponent를 통해 해당 타일의 소유 상태 업데이트
        if (WorldComponent)
        {
            if (UWorldTile* Tile = WorldComponent->GetTileAtHex(TileCoordinate))
            {
                Tile->SetOwned(false);
                Tile->SetOwnerPlayerID(-1);
            }
        }
    }
}


void ASuperPlayerState::ClearAllOwnedTiles(UWorldComponent* WorldComponent)
{
    // WorldComponent를 통해 모든 소유 타일의 소유 상태를 false로 설정
    if (WorldComponent)
    {
        for (const FVector2D& Coordinate : OwnedTileCoordinates)
        {
            if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
            {
                Tile->SetOwned(false);
                Tile->SetOwnerPlayerID(-1);
            }
        }
    }
    
    OwnedTileCoordinates.Empty();
    TotalOwnedTiles = 0;
}

TArray<UWorldTile*> ASuperPlayerState::GetOwnedTiles(UWorldComponent* WorldComponent) const
{
    TArray<UWorldTile*> OwnedTiles;
    
    if (!WorldComponent)
    {
        return OwnedTiles;
    }
    
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
        {
            OwnedTiles.Add(Tile);
        }
    }
    
    return OwnedTiles;
}

bool ASuperPlayerState::IsTileOwned(FVector2D TileCoordinate) const
{
    return OwnedTileCoordinates.Contains(TileCoordinate);
}


// ========== 매턴 처리 함수들 ==========
void ASuperPlayerState::ProcessTurnResources()
{
    // WorldComponent 참조 가져오기 (임시로 nullptr 처리, 나중에 GameMode나 GameInstance에서 가져올 예정)
    UWorldComponent* WorldComponent = nullptr;
    
    // TODO: GameMode나 GameInstance에서 WorldComponent 참조 가져오는 로직 추가 필요
    
    if (!WorldComponent)
    {
        return;
    }
    
    // 소유한 모든 타일의 생산량 계산
    int32 TurnFood = CalculateTotalFoodYield(WorldComponent);
    int32 TurnProduction = CalculateTotalProductionYield(WorldComponent);
    int32 TurnGold = CalculateTotalGoldYield(WorldComponent);
    int32 TurnScience = CalculateTotalScienceYield(WorldComponent);
    int32 TurnFaith = CalculateTotalFaithYield(WorldComponent);
    
    // 자원 추가
    AddFood(TurnFood);
    AddProduction(TurnProduction);
    AddGold(TurnGold);
    AddScience(TurnScience);
    AddFaith(TurnFaith);
    
}

void ASuperPlayerState::ProcessTurnPopulation()
{
    // 인구가 제한 인구 수에 도달했는지 확인
    if (Population >= LimitPopulation)
    {
        return;
    }
    
    // 데이터테이블에서 현재 인구 레벨에 맞는 식량 요구량 가져오기
    int32 RequiredFood = 100; // 기본값
    
    if (PopulationGrowthDataTable.IsValid())
    {
        UDataTable* DataTable = PopulationGrowthDataTable.LoadSynchronous();
        if (DataTable)
        {
            FPopulationGrowthData* GrowthData = DataTable->FindRow<FPopulationGrowthData>(
                *FString::Printf(TEXT("Population_%d"), Population), 
                TEXT("인구 증가 데이터 검색"));
            
            if (GrowthData)
            {
                RequiredFood = GrowthData->RequiredFood;
            }
        }
    }
    
    // 식량 게이지 최대값을 요구량으로 설정
    SetFoodGaugeMax(RequiredFood);
}


// ========== 자원 생산량 계산 함수들 ==========
int32 ASuperPlayerState::CalculateTotalFoodYield(UWorldComponent* WorldComponent) const
{
    int32 TotalFood = 0;
    
    if (!WorldComponent)
    {
        return 0;
    }
    
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
        {
            TotalFood += Tile->GetTotalFoodYield();
        }
    }
    
    return TotalFood;
}

int32 ASuperPlayerState::CalculateTotalProductionYield(UWorldComponent* WorldComponent) const
{
    int32 TotalProduction = 0;
    
    if (!WorldComponent)
    {
        return 0;
    }
    
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
        {
            TotalProduction += Tile->GetTotalProductionYield();
        }
    }
    
    return TotalProduction;
}

int32 ASuperPlayerState::CalculateTotalGoldYield(UWorldComponent* WorldComponent) const
{
    int32 TotalGold = 0;
    
    if (!WorldComponent)
    {
        return 0;
    }
    
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
        {
            TotalGold += Tile->GetTotalGoldYield();
        }
    }
    
    return TotalGold;
}

int32 ASuperPlayerState::CalculateTotalScienceYield(UWorldComponent* WorldComponent) const
{
    int32 TotalScience = 0;
    
    if (!WorldComponent)
    {
        return 0;
    }
    
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
        {
            TotalScience += Tile->GetTotalScienceYield();
        }
    }
    
    return TotalScience;
}

int32 ASuperPlayerState::CalculateTotalFaithYield(UWorldComponent* WorldComponent) const
{
    int32 TotalFaith = 0;
    
    if (!WorldComponent)
    {
        return 0;
    }
    
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
        {
            TotalFaith += Tile->GetTotalFaithYield();
        }
    }
    
    return TotalFaith;
}

// ========== 도시 관리 함수들 ==========
void ASuperPlayerState::SetCityCoordinate(FVector2D Coordinate)
{
    CityCoordinate = Coordinate;
    bHasCity = true;
}

void ASuperPlayerState::RemoveCity()
{
    CityCoordinate = FVector2D::ZeroVector;
    bHasCity = false;
}

// ========== 사치 자원 관리 함수들 ==========
void ASuperPlayerState::AddLuxuryResource(ELuxuryResource LuxuryResource, int32 Amount)
{
    if (LuxuryResource == ELuxuryResource::None || Amount <= 0)
    {
        return; // None 타입이거나 잘못된 개수는 추가하지 않음
    }
    
    int32* CurrentAmount = OwnedLuxuryResources.Find(LuxuryResource);
    if (CurrentAmount)
    {
        *CurrentAmount += Amount;
    }
    else
    {
        OwnedLuxuryResources.Add(LuxuryResource, Amount);
    }
    
    UpdatePopulationLimitFromLuxury();
    
}

void ASuperPlayerState::RemoveLuxuryResource(ELuxuryResource LuxuryResource, int32 Amount)
{
    if (LuxuryResource == ELuxuryResource::None || Amount <= 0)
    {
        return; // None 타입이거나 잘못된 개수는 제거하지 않음
    }
    
    int32* CurrentAmount = OwnedLuxuryResources.Find(LuxuryResource);
    if (CurrentAmount)
    {
        if (*CurrentAmount <= Amount)
        {
            // 제거할 개수가 현재 보유량보다 많거나 같으면 완전 제거
            OwnedLuxuryResources.Remove(LuxuryResource);
        }
        else
        {
            // 일부만 제거
            *CurrentAmount -= Amount;
        }
        
        UpdatePopulationLimitFromLuxury();
    }
}

void ASuperPlayerState::UpdatePopulationLimitFromLuxury()
{
    // 사치 자원 종류 수 계산 (Key 개수)
    int32 LuxuryTypeCount = OwnedLuxuryResources.Num();
    
    // 사치 자원 종류 1개당 인구 한계 +4
    int32 LuxuryBonus = LuxuryTypeCount * 4;
    LimitPopulation = 4 + LuxuryBonus; // 기본 4 + 사치 자원 보너스
}

int32 ASuperPlayerState::GetLuxuryResourceAmount(ELuxuryResource LuxuryResource) const
{
    const int32* Amount = OwnedLuxuryResources.Find(LuxuryResource);
    return Amount ? *Amount : 0;
}

bool ASuperPlayerState::HasLuxuryResource(ELuxuryResource LuxuryResource) const
{
    return OwnedLuxuryResources.Contains(LuxuryResource);
}

// ========== 게임 상태 함수들 ==========
void ASuperPlayerState::InitializePlayer()
{
    // 모든 자원 초기화
    FoodGauge = FResourceGauge(); // 기본값으로 초기화
    Production = 5;
    Gold = 20;
    Science = 0;
    Faith = 0;
    
    // 게임 진행 초기화
    Population = 1;
    LimitPopulation = 4;
    
    // 통계 초기화
    TotalFoodProduced = 0;
    TotalProductionProduced = 0;
    TotalGoldEarned = 0;
    TotalScienceGained = 0;
    TotalFaithGained = 0;
    
    // 타일 초기화 (좌표 배열만 비우기)
    OwnedTileCoordinates.Empty();
    TotalOwnedTiles = 0;
    
    // 도시 관리 초기화
    bHasCity = false;
    CityCoordinate = FVector2D::ZeroVector;
    
    // 사치 자원 관리 초기화
    OwnedLuxuryResources.Empty();
    UpdatePopulationLimitFromLuxury();
}
