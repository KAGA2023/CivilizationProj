// Fill out your copyright notice in the Description page of Project Settings.

#include "SuperPlayerState.h"
#include "WorldComponent.h"
#include "City/CityComponent.h"
#include "Unit/UnitCharacterBase.h"
#include "Unit/UnitManager.h"
#include "Engine/World.h"
#include "SuperGameInstance.h"

ASuperPlayerState::ASuperPlayerState()
{
    // 자원 초기화
    Food = 0;
    Production = 0;
    Gold = 0;
    Science = 0;
    Faith = 0;

    // 게임 진행 초기화
    Population = 1;
    LimitPopulation = 4;

    // 플레이어 인덱스 초기화
    PlayerIndex = -1;

    // 타일 관리 초기화
    TotalOwnedTiles = 0;
    OwnedTileCoordinates.Empty();

    // 도시 관리 초기화
    CityComponent = nullptr;
    bHasCity = false;
    CityCoordinate = FVector2D::ZeroVector;

    // 유닛 관리 초기화
    OwnedUnits.Empty();

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
        Food += Amount;
    }
}

void ASuperPlayerState::AddProduction(int32 Amount)
{
    if (Amount > 0)
    {
        Production += Amount;
    }
}

void ASuperPlayerState::AddGold(int32 Amount)
{
    if (Amount > 0)
    {
        Gold += Amount;
    }
}

void ASuperPlayerState::AddScience(int32 Amount)
{
    if (Amount > 0)
    {
        Science += Amount;
    }
}

void ASuperPlayerState::AddFaith(int32 Amount)
{
    if (Amount > 0)
    {
        Faith += Amount;
    }
}


bool ASuperPlayerState::SpendFood(int32 Amount)
{
    if (Amount <= 0 || Food < Amount)
    {
        return false;
    }
    
    Food -= Amount;
    return true;
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
                // PlayerIndex가 설정되어 있으면 PlayerIndex 사용, 없으면 GetPlayerId() 사용
                int32 OwnerID = (PlayerIndex >= 0) ? PlayerIndex : GetPlayerId();
                Tile->SetOwnerPlayerID(OwnerID);
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
    // GameInstance에서 WorldComponent 참조 가져오기
    UWorldComponent* WorldComponent = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            WorldComponent = GameInstance->GetGeneratedWorldComponent();
        }
    }
    
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
    
    // 도시 생산량 추가
    if (CityComponent)
    {
        TurnFood += CityComponent->GetFinalFoodYield();
        TurnProduction += CityComponent->GetFinalProductionYield();
        TurnGold += CityComponent->GetFinalGoldYield();
        TurnScience += CityComponent->GetFinalScienceYield();
        TurnFaith += CityComponent->GetFinalFaithYield();

        // 도시 건물 생산 진행도 업데이트 (식량과 생산력은 즉시 소비)
        CityComponent->UpdateBuildingProductionProgress(TurnFood, TurnProduction);

        // 도시 유닛 생산 진행도 업데이트 (식량과 생산력은 즉시 소비)
        CityComponent->UpdateUnitProductionProgress(TurnFood, TurnProduction);
        
        // 건물 생산 완료 확인
        FName CompletedBuildingName = CityComponent->CompleteBuildingProduction();
        if (CompletedBuildingName != NAME_None)
        {
            // 건물 생산 완료 (건물은 자동으로 추가됨)
        }
        
        // 유닛 생산 완료 확인 및 유닛 소환
        FName CompletedUnitName = CityComponent->CompleteUnitProduction();
        if (CompletedUnitName != NAME_None)
        {
            // 유닛 생산 완료 - 도시 좌표에 직접 소환
            if (UWorld* World = GetWorld())
            {
                if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
                {
                    if (UUnitManager* UnitManager = SuperGameInst->GetUnitManager())
                    {
                        // 도시 좌표에 직접 유닛 소환 (PlayerIndex 전달)
                        UnitManager->SpawnUnitAtHex(CityCoordinate, CompletedUnitName, PlayerIndex);
                    }
                }
            }
        }
    }
    
    // 자원 추가 (식량과 생산력은 건물/유닛 생산에 사용했으므로 추가하지 않음, 생산 중이 아닐 때만 추가)
    if (!CityComponent || CityComponent->GetCurrentStat().ProductionType == EProductionType::None)
    {
        AddFood(TurnFood);
        AddProduction(TurnProduction);
    }
    
    AddGold(TurnGold);
    AddScience(TurnScience);
    AddFaith(TurnFaith);
    
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
void ASuperPlayerState::SetCityComponent(UCityComponent* InCityComponent)
{
    CityComponent = InCityComponent;
    
    if (CityComponent)
    {
        bHasCity = true;
    }
    else
    {
        bHasCity = false;
    }
}

void ASuperPlayerState::SetCityCoordinate(FVector2D Coordinate)
{
    CityCoordinate = Coordinate;
}

void ASuperPlayerState::RemoveCity()
{
    CityComponent = nullptr;
    CityCoordinate = FVector2D::ZeroVector;
    bHasCity = false;
}

// ========== 유닛 관리 함수들 ==========
void ASuperPlayerState::AddOwnedUnit(AUnitCharacterBase* Unit)
{
    if (Unit && !OwnedUnits.Contains(Unit))
    {
        OwnedUnits.Add(Unit);
    }
}

void ASuperPlayerState::RemoveOwnedUnit(AUnitCharacterBase* Unit)
{
    if (Unit)
    {
        OwnedUnits.Remove(Unit);
    }
}

void ASuperPlayerState::ClearAllOwnedUnits()
{
    OwnedUnits.Empty();
}

// ========== 도시 건물 생산 함수들 ==========
bool ASuperPlayerState::StartBuildingProduction(FName BuildingRowName)
{
    // 도시가 없으면 실패
    if (!CityComponent)
    {
        return false;
    }

    // 건물 생산 시작/변경 (CityComponent에서 같은 건물이면 무시, 다른 생산 상태도 자동 리셋)
    CityComponent->StartBuildingProduction(BuildingRowName);
    return true;
}

// ========== 도시 유닛 생산 함수들 ==========
bool ASuperPlayerState::StartUnitProduction(FName UnitName)
{
    // 도시가 없으면 실패
    if (!CityComponent)
    {
        return false;
    }

    // 인구 제한 체크: Population이 LimitPopulation 이상이면 유닛 생산 불가
    if (Population >= LimitPopulation)
    {
        return false;
    }

    // 유닛 생산 시작/변경 (CityComponent에서 같은 유닛이면 무시, 다른 생산 상태도 자동 리셋)
    CityComponent->StartUnitProduction(UnitName);
    return true;
}

// ========== 도시 건물 구매 함수들 ==========
bool ASuperPlayerState::PurchaseBuildingWithGold(FName BuildingRowName)
{
    // 도시가 없으면 실패
    if (!CityComponent)
    {
        return false;
    }
    
    // 이미 건설된 건물이면 구매 불가 (RowName으로 중복 체크)
    if (CityComponent->HasBuilding(BuildingRowName))
    {
        return false;
    }
    
    // 건물 데이터 조회
    FBuildingData BuildingData = CityComponent->GetBuildingDataFromTable(BuildingRowName);
    if (BuildingData.BuildingType == EBuildingType::None)
    {
        return false;
    }
    
    // 구매 비용 확인
    int32 GoldCost = BuildingData.GoldCost;
    if (GoldCost <= 0 || Gold < GoldCost)
    {
        // 골드가 충분하지 않으면 실패
        return false;
    }
    
    // 골드 차감
    if (!SpendGold(GoldCost))
    {
        return false;
    }
    
    // 건물 추가
    CityComponent->AddBuilding(BuildingRowName);
    return true;
}

bool ASuperPlayerState::PurchaseBuildingWithFaith(FName BuildingRowName)
{
    // 도시가 없으면 실패
    if (!CityComponent)
    {
        return false;
    }
    
    // 이미 건설된 건물이면 구매 불가 (RowName으로 중복 체크)
    if (CityComponent->HasBuilding(BuildingRowName))
    {
        return false;
    }
    
    // 건물 데이터 조회
    FBuildingData BuildingData = CityComponent->GetBuildingDataFromTable(BuildingRowName);
    if (BuildingData.BuildingType == EBuildingType::None)
    {
        return false;
    }
    
    // 구매 비용 확인
    int32 FaithCost = BuildingData.FaithCost;
    if (FaithCost <= 0 || Faith < FaithCost)
    {
        // 신앙이 충분하지 않으면 실패
        return false;
    }
    
    // 신앙 차감
    if (!SpendFaith(FaithCost))
    {
        return false;
    }
    
    // 건물 추가
    CityComponent->AddBuilding(BuildingRowName);
    return true;
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
    Food = 0;
    Production = 5;
    Gold = 20;
    Science = 0;
    Faith = 0;
    
    // 게임 진행 초기화
    Population = 1;
    LimitPopulation = 4;
    
    // 플레이어 인덱스는 유지 (리셋 시 초기화하지 않음)
    
    // 타일 초기화 (좌표 배열만 비우기)
    OwnedTileCoordinates.Empty();
    TotalOwnedTiles = 0;
    
    // 도시 관리 초기화
    CityComponent = nullptr;
    bHasCity = false;
    CityCoordinate = FVector2D::ZeroVector;
    
    // 사치 자원 관리 초기화
    OwnedLuxuryResources.Empty();
    UpdatePopulationLimitFromLuxury();
}
