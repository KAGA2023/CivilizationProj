// Fill out your copyright notice in the Description page of Project Settings.

#include "SuperPlayerState.h"
#include "World/WorldComponent.h"
#include "City/CityComponent.h"
#include "Research/ResearchComponent.h"
#include "Research/Research.h"
#include "Unit/UnitCharacterBase.h"
#include "Unit/UnitManager.h"
#include "Facility/FacilityManager.h"
#include "Facility/FacilityStruct.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPath.h"
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

    // 전략 자원 관리 초기화
    OwnedStrategicResources.Empty();
    OwnedStrategicResourceStocks.Empty();

    // 시설 건설 가능 목록 초기화
    AvailableFacilities.Empty();
}

void ASuperPlayerState::BeginPlay()
{
    Super::BeginPlay();
    
    // 플레이어 초기화는 SuperGameModeBase에서 호출하므로 여기서는 호출하지 않음
    // InitializePlayer()는 SuperGameModeBase::InitializePlayers()에서 호출됨
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
        OnGoldChanged.Broadcast(Gold);
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
    OnGoldChanged.Broadcast(Gold);
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
    
    // 총 생산량 계산 (타일 + 도시 생산량 포함)
    int32 TurnFood = CalculateTotalFoodYield(WorldComponent);
    int32 TurnProduction = CalculateTotalProductionYield(WorldComponent);
    int32 TurnGold = CalculateTotalGoldYield(WorldComponent);
    int32 TurnScience = CalculateTotalScienceYield(WorldComponent);
    int32 TurnFaith = CalculateTotalFaithYield(WorldComponent);
    
    // 기술 컴포넌트 확인 및 연구 진행도 업데이트
    if (ResearchComponent)
    {
        // 기술 연구 진행도 업데이트 (과학량 사용)
        ResearchComponent->UpdateTechResearchProgress(TurnScience);
    }
    
    // 도시 컴포넌트 확인
    if (CityComponent)
    {
        // 현재 생산 상태 확인
        FCityCurrentStat CurrentStat = CityComponent->GetCurrentStat();
        
        // 건물 생산 진행도 업데이트 (생산력만 사용)
        if (CurrentStat.ProductionType == EProductionType::Building)
        {
            CityComponent->UpdateBuildingProductionProgress(TurnProduction);
        }
        
        // 유닛 생산 진행도 업데이트 (식량만 사용)
        FName CompletedUnitName = NAME_None;
        if (CurrentStat.ProductionType == EProductionType::Unit)
        {
            CompletedUnitName = CityComponent->UpdateUnitProductionProgress(TurnFood);
        }
        
        // 건물 생산 완료 확인
        FName CompletedBuildingName = CityComponent->CompleteBuildingProduction();
        if (CompletedBuildingName != NAME_None)
        {
            // 건물 생산 완료 (건물은 자동으로 추가됨)
        }
        
        // 유닛 생산 완료 확인 및 유닛 소환
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
                        AUnitCharacterBase* SpawnedUnit = UnitManager->SpawnUnitAtHex(CityCoordinate, CompletedUnitName, PlayerIndex);
                        if (SpawnedUnit)
                        {
                            // 유닛 소환 성공 - Population 증가
                            Population++;
                            OnPopulationChanged.Broadcast(Population);
                        }
                    }
                }
            }
        }
        
        // 생산 완료 후 다시 상태 확인 (생산이 완료되어 None이 되었을 수 있음)
        CurrentStat = CityComponent->GetCurrentStat();
        
        // 자원 추가 (생산 중인 타입에 따라 소비되는 자원이 다름)
        if (CurrentStat.ProductionType == EProductionType::Building)
        {
            // 건물 생산 중: 생산력 소비, 식량은 추가
            AddFood(TurnFood);
            // 생산력은 생산에 사용됨 (추가하지 않음)
        }
        else if (CurrentStat.ProductionType == EProductionType::Unit)
        {
            // 유닛 생산 중: 식량 소비, 생산력은 추가
            // 식량은 생산에 사용됨 (추가하지 않음)
            AddProduction(TurnProduction);
        }
        else
        {
            // 생산 중이 아님: 둘 다 추가
            AddFood(TurnFood);
            AddProduction(TurnProduction);
        }
    }
    else
    {
        // 도시가 없으면 모든 자원 추가
        AddFood(TurnFood);
        AddProduction(TurnProduction);
    }
    
    AddGold(TurnGold);
    AddScience(TurnScience);
    AddFaith(TurnFaith);
    
    // 전략 자원 보유량 증가 (시설이 있는 전략 자원만)
    for (const auto& Pair : OwnedStrategicResources)
    {
        EStrategicResource Resource = Pair.Key;
        if (Resource == EStrategicResource::None)
        {
            continue; // None은 건너뛰기
        }
        
        int32 CurrentStock = GetStrategicResourceStock(Resource);
        if (CurrentStock < 20) // 최대 20개 제한
        {
            int32 NewStock = FMath::Min(CurrentStock + 1, 20);
            OwnedStrategicResourceStocks.Add(Resource, NewStock);
            OnStrategicResourceStockChanged.Broadcast(Resource, NewStock);
        }
    }
}

// ========== 자원 생산량 계산 함수들 ==========
int32 ASuperPlayerState::CalculateTotalFoodYield(UWorldComponent* WorldComponent) const
{
    int32 TotalFood = 0;
    
    if (!WorldComponent)
    {
        return 0;
    }
    
    // 타일 생산량 계산
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
        {
            TotalFood += Tile->GetTotalFoodYield();
        }
    }
    
    // 도시 생산량 추가 (건물 보너스 포함)
    if (CityComponent)
    {
        TotalFood += CityComponent->GetFinalFoodYield();
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
    
    // 타일 생산량 계산
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
        {
            TotalProduction += Tile->GetTotalProductionYield();
        }
    }
    
    // 도시 생산량 추가 (건물 보너스 포함)
    if (CityComponent)
    {
        TotalProduction += CityComponent->GetFinalProductionYield();
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
    
    // 타일 생산량 계산
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
        {
            TotalGold += Tile->GetTotalGoldYield();
        }
    }
    
    // 도시 생산량 추가 (건물 보너스 포함)
    if (CityComponent)
    {
        TotalGold += CityComponent->GetFinalGoldYield();
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
    
    // 타일 생산량 계산
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
        {
            TotalScience += Tile->GetTotalScienceYield();
        }
    }
    
    // 도시 생산량 추가 (건물 보너스 포함)
    if (CityComponent)
    {
        TotalScience += CityComponent->GetFinalScienceYield();
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
    
    // 타일 생산량 계산
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (UWorldTile* Tile = WorldComponent->GetTileAtHex(Coordinate))
        {
            TotalFaith += Tile->GetTotalFaithYield();
        }
    }
    
    // 도시 생산량 추가 (건물 보너스 포함)
    if (CityComponent)
    {
        TotalFaith += CityComponent->GetFinalFaithYield();
    }
    
    return TotalFaith;
}

// ========== 도시 관리 함수들 ==========
void ASuperPlayerState::SetCityComponent(UCityComponent* InCityComponent)
{
    CityComponent = InCityComponent;
    
    if (CityComponent)
    {
        // CityComponent 초기화
        FCityData CityData;
        CityData.CityName = TEXT("New City");
        CityData.MaxHealth = 100;
        CityData.FoodYield = 2;
        CityData.ProductionYield = 1;
        CityData.GoldYield = 1;
        CityData.ScienceYield = 0;
        CityData.FaithYield = 0;
        CityData.BuiltBuildings.Empty();
        
        CityComponent->InitFromCityData(CityData);
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

// ========== 기술 관리 함수들 ==========
void ASuperPlayerState::SetResearchComponent(UResearchComponent* InResearchComponent)
{
    ResearchComponent = InResearchComponent;
    
    if (ResearchComponent)
    {
        // ResearchComponent 초기화
        FResearchData ResearchData;
        ResearchData.ResearchedTechs.Empty();
        ResearchComponent->InitFromResearchData(ResearchData);
    }
}

// ========== 기술 연구 함수들 ==========
bool ASuperPlayerState::StartTechResearch(FName TechRowName)
{
    if (!ResearchComponent)
    {
        return false;
    }

    // 기술 연구 시작/변경 (ResearchComponent에서 같은 기술이면 무시, 다른 연구 상태도 자동 리셋)
    ResearchComponent->StartTechResearch(TechRowName);
    return true;
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
    if (Unit && OwnedUnits.Contains(Unit))
    {
        OwnedUnits.Remove(Unit);
        
        // Population 감소 및 델리게이트 브로드캐스트
        if (Population > 0)
        {
            Population--;
            OnPopulationChanged.Broadcast(Population);
        }
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

    // 유닛 데이터 조회
    FUnitBaseStat UnitStat = CityComponent->GetUnitDataFromTable(UnitName);
    if (UnitStat.UnitClass == EUnitClass::None)
    {
        return false;
    }

    // 전략 자원 체크 및 소모
    for (int32 i = 0; i < UnitStat.RequiredResources.Num(); i++)
    {
        EStrategicResource Resource = UnitStat.RequiredResources[i];
        int32 RequiredAmount = UnitStat.RequiredResourceAmounts[i];
        
        if (!CanAffordStrategicResource(Resource, RequiredAmount))
        {
            return false; // 부족하면 실패
        }
    }

    // 모두 충분하면 소모
    for (int32 i = 0; i < UnitStat.RequiredResources.Num(); i++)
    {
        EStrategicResource Resource = UnitStat.RequiredResources[i];
        int32 RequiredAmount = UnitStat.RequiredResourceAmounts[i];
        SpendStrategicResource(Resource, RequiredAmount);
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

// ========== 도시 유닛 구매 함수들 ==========
bool ASuperPlayerState::PurchaseUnitWithGold(FName UnitName)
{
    // 도시가 없으면 실패
    if (!CityComponent)
    {
        return false;
    }
    
    // 인구 제한 체크: Population이 LimitPopulation 이상이면 유닛 구매 불가
    if (Population >= LimitPopulation)
    {
        return false;
    }
    
    // 유닛 데이터 조회
    FUnitBaseStat UnitStat = CityComponent->GetUnitDataFromTable(UnitName);
    if (UnitStat.UnitClass == EUnitClass::None)
    {
        return false;
    }
    
    // 구매 비용 확인
    int32 GoldCost = UnitStat.GoldCost;
    if (GoldCost <= 0 || Gold < GoldCost)
    {
        // 골드가 충분하지 않으면 실패
        return false;
    }

    // 전략 자원 체크
    for (int32 i = 0; i < UnitStat.RequiredResources.Num(); i++)
    {
        EStrategicResource Resource = UnitStat.RequiredResources[i];
        int32 RequiredAmount = UnitStat.RequiredResourceAmounts[i];
        
        if (!CanAffordStrategicResource(Resource, RequiredAmount))
        {
            return false; // 부족하면 실패
        }
    }
    
    // 골드 차감
    if (!SpendGold(GoldCost))
    {
        return false;
    }

    // 전략 자원 소모
    for (int32 i = 0; i < UnitStat.RequiredResources.Num(); i++)
    {
        EStrategicResource Resource = UnitStat.RequiredResources[i];
        int32 RequiredAmount = UnitStat.RequiredResourceAmounts[i];
        SpendStrategicResource(Resource, RequiredAmount);
    }
    
    // 유닛 소환 (도시 좌표에 소환, PlayerIndex 전달)
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            if (UUnitManager* UnitManager = SuperGameInst->GetUnitManager())
            {
                // 도시 좌표에 유닛 소환 (PlayerIndex 전달하여 올바른 소유자 설정)
                AUnitCharacterBase* SpawnedUnit = UnitManager->SpawnUnitAtHex(CityCoordinate, UnitName, PlayerIndex);
                if (SpawnedUnit)
                {
                    // 유닛 소환 성공 - Population 증가
                    Population++;
                    OnPopulationChanged.Broadcast(Population);
                    return true;
                }
                else
                {
                    // 유닛 소환 실패 시 골드 환불
                    AddGold(GoldCost);
                    return false;
                }
            }
        }
    }
    
    // 유닛 소환 실패 시 골드 환불
    AddGold(GoldCost);
    return false;
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

// ========== 전략 자원 관리 함수들 ==========
void ASuperPlayerState::AddStrategicResource(EStrategicResource StrategicResource, int32 Amount)
{
    if (StrategicResource == EStrategicResource::None || Amount <= 0)
    {
        return; // None 타입이거나 잘못된 개수는 추가하지 않음
    }
    
    int32* CurrentAmount = OwnedStrategicResources.Find(StrategicResource);
    if (CurrentAmount)
    {
        *CurrentAmount += Amount;
    }
    else
    {
        OwnedStrategicResources.Add(StrategicResource, Amount);
    }
}

void ASuperPlayerState::RemoveStrategicResource(EStrategicResource StrategicResource, int32 Amount)
{
    if (StrategicResource == EStrategicResource::None || Amount <= 0)
    {
        return; // None 타입이거나 잘못된 개수는 제거하지 않음
    }
    
    int32* CurrentAmount = OwnedStrategicResources.Find(StrategicResource);
    if (CurrentAmount)
    {
        if (*CurrentAmount <= Amount)
        {
            // 제거할 개수가 현재 보유량보다 많거나 같으면 완전 제거
            OwnedStrategicResources.Remove(StrategicResource);
        }
        else
        {
            // 일부만 제거
            *CurrentAmount -= Amount;
        }
    }
}

int32 ASuperPlayerState::GetStrategicResourceAmount(EStrategicResource StrategicResource) const
{
    const int32* Amount = OwnedStrategicResources.Find(StrategicResource);
    return Amount ? *Amount : 0;
}

bool ASuperPlayerState::HasStrategicResource(EStrategicResource StrategicResource) const
{
    return OwnedStrategicResources.Contains(StrategicResource);
}

int32 ASuperPlayerState::GetStrategicResourceStock(EStrategicResource StrategicResource) const
{
    const int32* Stock = OwnedStrategicResourceStocks.Find(StrategicResource);
    return Stock ? *Stock : 0;
}

bool ASuperPlayerState::CanAffordStrategicResource(EStrategicResource StrategicResource, int32 Amount) const
{
    int32 CurrentStock = GetStrategicResourceStock(StrategicResource);
    return CurrentStock >= Amount;
}

bool ASuperPlayerState::SpendStrategicResource(EStrategicResource StrategicResource, int32 Amount)
{
    if (StrategicResource == EStrategicResource::None || Amount <= 0)
    {
        return false;
    }

    int32 CurrentStock = GetStrategicResourceStock(StrategicResource);
    if (CurrentStock < Amount)
    {
        return false; // 부족하면 실패
    }

    int32 NewStock = CurrentStock - Amount;
    OwnedStrategicResourceStocks.Add(StrategicResource, NewStock);
    OnStrategicResourceStockChanged.Broadcast(StrategicResource, NewStock);
    return true;
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

    // 전략 자원 관리 초기화
    OwnedStrategicResources.Empty();
    OwnedStrategicResourceStocks.Empty();

    // 시설 건설 가능 목록 업데이트
    UpdateAvailableFacilities();
}

// ========== 시설 건설 함수들 ==========
void ASuperPlayerState::UpdateAvailableFacilities()
{
    // 데이터 테이블이 로드되지 않았으면 로드 시도
    if (!FacilityDataTable)
    {
        FSoftObjectPath FacilityDataTablePath(TEXT("/Game/Civilization/Data/DT_FacilityData.DT_FacilityData"));
        FacilityDataTable = Cast<UDataTable>(FacilityDataTablePath.TryLoad());
        
        if (!FacilityDataTable)
        {
            return;
        }
    }
    
    // AvailableFacilities 비우기
    AvailableFacilities.Empty();

    // 데이터 테이블에서 모든 시설 RowName 가져오기
    TArray<FName> AllRowNames = FacilityDataTable->GetRowNames();

    // ResearchComponent 가져오기
    if (!ResearchComponent)
    {
        // ResearchComponent가 없으면 모든 시설 추가 (기본 동작)
        AvailableFacilities = AllRowNames;
        return;
    }

    // 각 시설에 대해 필터링
    for (const FName& RowName : AllRowNames)
    {
        FFacilityData* FacilityData = FacilityDataTable->FindRow<FFacilityData>(RowName, TEXT("UpdateAvailableFacilities"));
        if (!FacilityData)
        {
            continue;
        }

        // 기술 조건 확인 (ResearchComponent 헬퍼 함수 사용)
        if (ResearchComponent->IsFacilityUnlocked(RowName))
        {
            AvailableFacilities.Add(RowName);
        }
    }
}

TArray<FFacilityData> ASuperPlayerState::GetBuildableFacilities(UWorldTile* TargetTile) const
{
    TArray<FFacilityData> BuildableFacilities;

    // 타일이 유효한지 확인
    if (!TargetTile)
    {
        return BuildableFacilities;
    }

    // FacilityManager 가져오기
    UFacilityManager* FacilityManager = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            FacilityManager = GameInstance->GetFacilityManager();
        }
    }

    if (!FacilityManager || !FacilityManager->FacilityDataTable)
    {
        return BuildableFacilities;
    }

    // AvailableFacilities를 기준으로 필터링
    for (const FName& RowName : AvailableFacilities)
    {
        FFacilityData* FacilityData = FacilityManager->FacilityDataTable->FindRow<FFacilityData>(RowName, TEXT("GetBuildableFacilities"));
        if (!FacilityData)
        {
            continue;
        }

        // 타일 조건 확인 (FacilityManager::CanBuildFacilityOnTile 호출)
        if (FacilityManager->CanBuildFacilityOnTile(RowName, TargetTile))
        {
            BuildableFacilities.Add(*FacilityData);
        }
    }

    return BuildableFacilities;
}

bool ASuperPlayerState::BuildFacility(FName FacilityRowName, FVector2D TileCoordinate)
{
    // FacilityManager 및 WorldComponent 가져오기
    UFacilityManager* FacilityManager = nullptr;
    UWorldComponent* WorldComponent = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            FacilityManager = GameInstance->GetFacilityManager();
            WorldComponent = GameInstance->GetGeneratedWorldComponent();
        }
    }

    if (!FacilityManager || !WorldComponent)
    {
        return false;
    }

    // 최종 실행 (건설 + 모디파이어 적용 + 건설자 제거 + 델리게이트 브로드캐스트)
    return FacilityManager->BuildFacility(FacilityRowName, TileCoordinate, WorldComponent);
}
