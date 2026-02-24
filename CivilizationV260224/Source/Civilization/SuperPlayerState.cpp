// Fill out your copyright notice in the Description page of Project Settings.

#include "SuperPlayerState.h"
#include "World/WorldComponent.h"
#include "City/CityComponent.h"
#include "Research/ResearchComponent.h"
#include "Research/Research.h"
#include "Unit/UnitCharacterBase.h"
#include "Unit/UnitManager.h"
#include "Facility/FacilityManager.h"
#include "Facility/FacilityActor.h"
#include "Facility/FacilityStruct.h"
#include "Border/BorderManager.h"
#include "Diplomacy/DiplomacyManager.h"
#include "Country/CountryStruct.h"
#include "Engine/World.h"
#include "Engine/DataTable.h"
#include "UObject/SoftObjectPath.h"
#include "SuperGameInstance.h"
#include "SuperGameModeBase.h"
#include "World/WorldSpawner.h"
#include "Kismet/GameplayStatics.h"

ASuperPlayerState::ASuperPlayerState()
{
    // 자원 초기화
    Food = 0;
    Production = 0;
    Gold = 0;
    Science = 0;
    Faith = 0;

    // 게임 진행 초기화
    Population = 0;
    LimitPopulation = 4;

    // 플레이어 인덱스 초기화
    PlayerIndex = -1;

    // 국가 정보 초기화
    CountryName = TEXT("");
    CountryLargeImg = nullptr;
    CountryKingImg = nullptr;
    BorderColor = FLinearColor::White;

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
        
        // 타일 제거 후 국경선 업데이트
        if (UWorld* World = GetWorld())
        {
            if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
            {
                if (UBorderManager* BorderManager = SuperGameInst->GetBorderManager())
                {
                    // 현재 플레이어의 국경선 업데이트
                    TArray<FVector2D> OwnedTiles = GetOwnedTileCoordinates();
                    BorderManager->UpdatePlayerBorder(PlayerIndex, OwnedTiles);
                }
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

// ========== 타일 구매 시스템 ==========
bool ASuperPlayerState::CanPurchaseTile(FVector2D TileCoordinate, UWorldComponent* WorldComponent)
{
    // WorldComponent가 유효한지 확인
    if (!WorldComponent)
    {
        return false;
    }
    
    // 타일이 존재하는지 확인
    UWorldTile* Tile = WorldComponent->GetTileAtHex(TileCoordinate);
    if (!Tile)
    {
        return false;
    }
    
    // 이미 소유한 타일이면 구매 불가
    if (IsTileOwned(TileCoordinate))
    {
        return false;
    }
    
    // 다른 플레이어가 소유한 타일이면 구매 불가
    int32 TileOwnerID = Tile->GetOwnerPlayerID();
    if (TileOwnerID != -1 && TileOwnerID != PlayerIndex)
    {
        return false;
    }
    
    // 내가 소유한 타일과 거리 1인 타일만 구매 가능
    // 인접 타일 좌표들을 가져옴
    TArray<FVector2D> NeighborCoords = WorldComponent->GetHexNeighbors(TileCoordinate);
    
    // 인접 타일 중 하나라도 내가 소유한 타일이 있는지 확인
    bool bIsAdjacentToOwnedTile = false;
    for (const FVector2D& NeighborCoord : NeighborCoords)
    {
        if (IsTileOwned(NeighborCoord))
        {
            bIsAdjacentToOwnedTile = true;
            break;
        }
    }
    
    // 인접한 소유 타일이 없으면 구매 불가
    if (!bIsAdjacentToOwnedTile)
    {
        return false;
    }
    
    return true;
}

int32 ASuperPlayerState::CalculateTilePurchaseCost(FVector2D TileCoordinate, UWorldComponent* WorldComponent)
{
    // WorldComponent가 유효한지 확인
    if (!WorldComponent)
    {
        return 0;
    }
    
    // 도시가 없으면 비용 계산 불가 (또는 기본값 반환)
    if (!HasCity())
    {
        return 0; // 또는 기본 비용 반환 가능
    }
    
    // 도시 좌표 가져오기
    FVector2D CityCoord = GetCityCoordinate();
    
    // 도시와 타일 간 거리 계산
    int32 Distance = WorldComponent->GetHexDistance(CityCoord, TileCoordinate);
    
    // 비용 계산: 기본 25골드 + 거리당 5골드
    int32 BaseCost = 25;
    int32 CostPerDistance = 5;
    int32 TotalCost = BaseCost + (Distance * CostPerDistance);
    
    return TotalCost;
}

bool ASuperPlayerState::PurchaseTile(FVector2D TileCoordinate, UWorldComponent* WorldComponent)
{
    // 구매 가능 여부 확인
    if (!CanPurchaseTile(TileCoordinate, WorldComponent))
    {
        return false;
    }
    
    // 구매 비용 계산
    int32 Cost = CalculateTilePurchaseCost(TileCoordinate, WorldComponent);
    if (Cost <= 0)
    {
        return false; // 비용이 0 이하면 구매 불가
    }
    
    // 골드가 충분한지 확인 및 차감
    if (!SpendGold(Cost))
    {
        return false; // 골드가 부족하면 구매 실패
    }
    
    // 타일 소유 목록에 추가 (AddOwnedTile이 내부에서 소유 상태 업데이트까지 처리)
    AddOwnedTile(TileCoordinate, WorldComponent);
    
    // 타일 구매 성공 후 국경선 업데이트
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            if (UBorderManager* BorderManager = SuperGameInst->GetBorderManager())
            {
                // 현재 플레이어의 국경선 업데이트
                TArray<FVector2D> OwnedTiles = GetOwnedTileCoordinates();
                BorderManager->UpdatePlayerBorder(PlayerIndex, OwnedTiles);
            }
        }
    }
    
    // 타일 구매 성공 후 MainHUD 업데이트를 위해 OnGoldChanged 다시 브로드캐스트 (생산량 변경 반영)
    OnGoldChanged.Broadcast(Gold);
    
    // 구매 성공
    return true;
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
                        // 도시 주변 1칸 내에서 소환 가능한 타일 찾기
                        FVector2D SpawnHex = UnitManager->FindSpawnLocationNearCity(CityCoordinate);
                        if (SpawnHex != FVector2D(-1, -1))
                        {
                            // 찾은 타일에 유닛 소환 (PlayerIndex 전달)
                            AUnitCharacterBase* SpawnedUnit = UnitManager->SpawnUnitAtHex(SpawnHex, CompletedUnitName, PlayerIndex);
                            if (SpawnedUnit)
                            {
                                // 유닛 소환 성공 - Population 증가
                                Population++;
                                OnPopulationChanged.Broadcast(Population);
                            }
                        }
                        // SpawnHex가 -1이어도 특별한 처리 없음 (다음 턴에 다시 시도 가능)
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

    UFacilityManager* FacilityManager = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            FacilityManager = GameInstance->GetFacilityManager();
        }
    }
    
    // 타일 생산량 계산 (약탈된 시설 타일은 0)
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (FacilityManager && FacilityManager->HasFacilityAtTile(Coordinate))
        {
            if (AFacilityActor* FacilityActor = FacilityManager->GetFacilityActorAtHex(Coordinate))
            {
                if (FacilityActor->bIsPillaged)
                {
                    continue;
                }
            }
        }
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

    UFacilityManager* FacilityManager = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            FacilityManager = GameInstance->GetFacilityManager();
        }
    }
    
    // 타일 생산량 계산 (약탈된 시설 타일은 0)
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (FacilityManager && FacilityManager->HasFacilityAtTile(Coordinate))
        {
            if (AFacilityActor* FacilityActor = FacilityManager->GetFacilityActorAtHex(Coordinate))
            {
                if (FacilityActor->bIsPillaged)
                {
                    continue;
                }
            }
        }
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

    UFacilityManager* FacilityManager = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            FacilityManager = GameInstance->GetFacilityManager();
        }
    }
    
    // 타일 생산량 계산 (약탈된 시설 타일은 0)
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (FacilityManager && FacilityManager->HasFacilityAtTile(Coordinate))
        {
            if (AFacilityActor* FacilityActor = FacilityManager->GetFacilityActorAtHex(Coordinate))
            {
                if (FacilityActor->bIsPillaged)
                {
                    continue;
                }
            }
        }
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

    UFacilityManager* FacilityManager = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            FacilityManager = GameInstance->GetFacilityManager();
        }
    }
    
    // 타일 생산량 계산 (약탈된 시설 타일은 0)
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (FacilityManager && FacilityManager->HasFacilityAtTile(Coordinate))
        {
            if (AFacilityActor* FacilityActor = FacilityManager->GetFacilityActorAtHex(Coordinate))
            {
                if (FacilityActor->bIsPillaged)
                {
                    continue;
                }
            }
        }
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

    UFacilityManager* FacilityManager = nullptr;
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            FacilityManager = GameInstance->GetFacilityManager();
        }
    }
    
    // 타일 생산량 계산 (약탈된 시설 타일은 0)
    for (const FVector2D& Coordinate : OwnedTileCoordinates)
    {
        if (FacilityManager && FacilityManager->HasFacilityAtTile(Coordinate))
        {
            if (AFacilityActor* FacilityActor = FacilityManager->GetFacilityActorAtHex(Coordinate))
            {
                if (FacilityActor->bIsPillaged)
                {
                    continue;
                }
            }
        }
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
        
        // 기본 기술 추가 (게임 시작 시)
        ResearchData.ResearchedTechs.Add(FName("StartTech"));
        
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
                // 도시 주변 1칸 내에서 소환 가능한 타일 찾기
                FVector2D SpawnHex = UnitManager->FindSpawnLocationNearCity(CityCoordinate);
                if (SpawnHex != FVector2D(-1, -1))
                {
                    // 찾은 타일에 유닛 소환 (PlayerIndex 전달하여 올바른 소유자 설정)
                    AUnitCharacterBase* SpawnedUnit = UnitManager->SpawnUnitAtHex(SpawnHex, UnitName, PlayerIndex);
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
                else
                {
                    // 소환 가능한 타일을 찾지 못함 → 골드 환불
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
    Gold = 1000;
    Science = 0;
    Faith = 0;
    
    // 게임 진행 초기화
    Population = 0;
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
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            if (UUnitManager* UnitManager = GameInstance->GetUnitManager())
            {
                UnitManager->RequestBuilderBuildFacility(TileCoordinate, FacilityRowName);
                return true;
            }
        }
    }
    return false;
}

// ========== 국가 정보 함수들 ==========
void ASuperPlayerState::LoadCountryDataFromTable()
{
    // CountryRowName이 없으면 로드 불가
    if (CountryRowName == NAME_None)
    {
        return;
    }

    // GameInstance에서 CountryDataTable 가져오기
    if (!GetWorld())
    {
        return;
    }

    USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(GetWorld()->GetGameInstance());
    if (!SuperGameInst)
    {
        return;
    }

    UDataTable* CountryDataTable = SuperGameInst->GetCountryDataTable();
    if (!CountryDataTable)
    {
        return;
    }

    // 데이터 테이블에서 국가 데이터 조회
    FCountryData* CountryData = CountryDataTable->FindRow<FCountryData>(CountryRowName, TEXT("LoadCountryDataFromTable"));
    if (!CountryData)
    {
        return;
    }

    // 조회한 데이터를 캐시에 저장
    CountryName = CountryData->CountryName;
    CountryLargeImg = CountryData->CountryLargeImg;
    CountryKingImg = CountryData->CountryKingImg;
    BorderColor = CountryData->BorderColor;
}

// ========== 승리/패배 시스템 구현 ==========
void ASuperPlayerState::SetDefeated()
{
    if (bIsDefeated)
    {
        return; // 이미 패배 상태
    }
    
    bIsDefeated = true;
    
    // 플레이어 0인가 AI인가에 따라 다르게 처리
    if (PlayerIndex == 0)
    {
        // 플레이어(인간) 패배 → 패배 위젯 표시
        OnPlayerDefeated_Human();
        // CheckGameEndConditions() 호출 안 함 (이미 패배)
    }
    else
    {
        // AI 패배 → AI 패배 위젯 표시 + 정리
        OnPlayerDefeated_AI();
        
        // AI 패배 시에만 승리 조건 체크!
        if (UWorld* World = GetWorld())
        {
            if (ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode()))
            {
                GameMode->CheckGameEndConditions();
            }
        }
    }
}

void ASuperPlayerState::OnPlayerDefeated_Human()
{
    // 플레이어 패배 델리게이트 브로드캐스트
    OnPlayerDefeatedDelegate.Broadcast();
}

void ASuperPlayerState::OnPlayerDefeated_AI()
{
	// Player 0에게 AI 패배 알림 (MainHUD가 Player 0의 델리게이트를 듣고 있음)
	if (UWorld* World = GetWorld())
	{
		if (USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance()))
		{
			if (ASuperPlayerState* Player0 = GameInstance->GetPlayerState(0))
			{
				Player0->OnAIPlayerDefeatedDelegate.Broadcast(PlayerIndex);
			}
		}
	}
	
	// 패배한 AI 정리 (유닛 제거, 영토 중립화)
	CleanupDefeatedPlayer();
}

void ASuperPlayerState::CleanupDefeatedPlayer()
{
    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }
    
    USuperGameInstance* GameInstance = Cast<USuperGameInstance>(World->GetGameInstance());
    if (!GameInstance)
    {
        return;
    }
    
    // 1. 모든 유닛 제거
    UUnitManager* UnitManager = GameInstance->GetUnitManager();
    if (UnitManager)
    {
        // OwnedUnits 배열을 복사 (순회 중 배열이 변경되므로)
        TArray<AUnitCharacterBase*> UnitsToRemove = OwnedUnits;
        
        for (AUnitCharacterBase* Unit : UnitsToRemove)
        {
            if (Unit)
            {
                FVector2D UnitHex = UnitManager->GetHexPositionForUnit(Unit);
                UnitManager->DestroyUnit(Unit, UnitHex);
            }
        }
        
        // 모든 유닛 제거 확인
        OwnedUnits.Empty();
    }
    
    // 2. 모든 영토 중립화
    UWorldComponent* WorldComp = GameInstance->GetGeneratedWorldComponent();
    if (WorldComp)
    {
        // OwnedTileCoordinates 배열을 복사
        TArray<FVector2D> TilesToClear = OwnedTileCoordinates;
        
        for (FVector2D TileCoord : TilesToClear)
        {
            UWorldTile* Tile = WorldComp->GetTileAtHex(TileCoord);
            if (Tile)
            {
                Tile->SetOwnerPlayerID(-1); // 중립으로 설정
            }
        }
        
        // 소유 타일 목록 비우기
        OwnedTileCoordinates.Empty();
        TotalOwnedTiles = 0;
    }
    
    // 3. 국경선 제거
    UBorderManager* BorderManager = GameInstance->GetBorderManager();
    if (BorderManager)
    {
        BorderManager->RemovePlayerBorder(PlayerIndex);
    }
    
    // 4. 외교 관계 정리
    UDiplomacyManager* DiplomacyManager = GameInstance->GetDiplomacyManager();
    if (DiplomacyManager)
    {
        DiplomacyManager->CleanupPlayerDiplomacy(PlayerIndex);
    }
    
    // 5. 도시 액터 파괴 및 도시 데이터 정리
    if (bHasCity && CityComponent)
    {
        // 헥스 좌표를 정수로 보정 (소수점으로 인한 조회 실패 방지)
        const FVector2D CityHex(static_cast<float>(FMath::RoundToInt(CityCoordinate.X)), static_cast<float>(FMath::RoundToInt(CityCoordinate.Y)));
        
        // 도시 생산 큐 중단
        CityComponent->StopCurrentProduction();
        
        // 월드에서 도시 액터 파괴 (함수 상단에서 얻은 World 사용)
        if (AWorldSpawner* WorldSpawner = Cast<AWorldSpawner>(UGameplayStatics::GetActorOfClass(World, AWorldSpawner::StaticClass())))
        {
            WorldSpawner->DestroyCityActorAtHex(CityHex);
        }
        
        // WorldComponent의 시작 도시 목록에서 제거
        if (WorldComp)
        {
            WorldComp->RemoveCityAt(CityHex);
        }
        
        RemoveCity();
    }
    
    // 6. 연구 컴포넌트 정리
    if (ResearchComponent)
    {
        // 연구 중단
        ResearchComponent->StopCurrentResearch();
    }
    
    // 7. 자원 초기화
    Food = 0;
    Production = 0;
    Gold = 0;
    Science = 0;
    Faith = 0;
    
    // 8. 사치/전략 자원 초기화
    OwnedLuxuryResources.Empty();
    OwnedStrategicResources.Empty();
    OwnedStrategicResourceStocks.Empty();
    
    // 9. 인구 초기화
    Population = 0;
    LimitPopulation = 4; // 기본값
    
    // 10. 시설 목록 초기화
    AvailableFacilities.Empty();
}
