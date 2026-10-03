// Fill out your copyright notice in the Description page of Project Settings.

#include "SaveLoadManager.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "../SuperGameInstance.h"
#include "../SuperPlayerState.h"
#include "../City/CityComponent.h"
#include "../City/CityActor.h"
#include "../Unit/UnitCharacterBase.h"
#include "../Unit/UnitManager.h"
#include "../Widget/UnitWidget/SmallUnitUI.h"
#include "../Status/UnitStatusComponent.h"
#include "../World/WorldComponent.h"
#include "../World/WorldStruct.h"
#include "../World/WorldSpawner.h"
#include "../Facility/FacilityManager.h"
#include "../Diplomacy/DiplomacyManager.h"
#include "../Turn/TurnComponent.h"
#include "../SuperGameModeBase.h"
#include "../Research/ResearchComponent.h"

USaveLoadManager::USaveLoadManager()
{
    GameInstance = nullptr;
}

void USaveLoadManager::SetGameInstance(USuperGameInstance* InGameInstance)
{
    GameInstance = InGameInstance;
}

// ========== 세이브 함수 ==========
bool USaveLoadManager::CollectGameStateForSave(FGameSaveData& OutSaveData)
{
    if (!GameInstance)
    {
        return false;
    }

    // 게임 메타 정보
    OutSaveData.SaveDateTime = FDateTime::Now();

    // WorldConfig 수집 (메인메뉴에서 로드 시 월드 생성에 필요)
    OutSaveData.WorldConfig = GameInstance->GetWorldConfig();

    // 라운드 정보 수집 (세이브는 항상 플레이어 턴에서만 하므로 라운드 번호만 저장)
    UWorld* World = GameInstance->GetWorld();
    if (!World)
    {
        return false;
    }

    ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode());
    if (GameMode && GameMode->GetTurnComponent())
    {
        OutSaveData.CurrentRound = GameMode->GetTurnComponent()->GetCurrentRoundNumber();
    }

    // 플레이어 데이터 수집 (도시 데이터 및 유닛 데이터 포함)
    OutSaveData.PlayerDataArray.Empty();
    for (int32 i = 0; i < GameInstance->GetPlayerStateCount(); i++)
    {
        ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(i);
        if (PlayerState)
        {
            FPlayerSaveData PlayerData;
            CollectPlayerData(PlayerState, PlayerData);
            
            // 해당 플레이어의 유닛 데이터 수집
            PlayerData.UnitDataArray.Empty();
            if (GameInstance->GetUnitManager())
            {
                TArray<AUnitCharacterBase*> AllUnits = GameInstance->GetUnitManager()->GetAllUnits();
                for (AUnitCharacterBase* Unit : AllUnits)
                {
                    if (Unit && Unit->GetPlayerIndex() == PlayerState->PlayerIndex)
                    {
                        FUnitSaveData UnitData;
                        CollectUnitData(Unit, UnitData);
                        PlayerData.UnitDataArray.Add(UnitData);
                    }
                }
            }
            
            OutSaveData.PlayerDataArray.Add(PlayerData);
        }
    }

    // 타일 소유권은 각 FPlayerSaveData의 OwnedTileCoordinates에 저장되므로 별도로 저장하지 않음

    // 월드 타일 데이터 수집 (지형, 기후, 자원, 숲, 시설 정보)
    OutSaveData.WorldDataMap.Empty();
    if (GameInstance->GetGeneratedWorldComponent() && GameInstance->GetFacilityManager())
    {
        CollectWorldData(GameInstance->GetGeneratedWorldComponent(), GameInstance->GetFacilityManager(), OutSaveData.WorldDataMap);
    }

    // 외교 데이터 수집
    OutSaveData.DiplomacyStateMap.Empty();
    OutSaveData.DiplomacyActionHistory.Empty();
    OutSaveData.Attitudes.Empty();
    OutSaveData.NextActionId = 1;
    if (GameInstance->GetDiplomacyManager())
    {
        CollectDiplomacyData(GameInstance->GetDiplomacyManager(), OutSaveData.DiplomacyStateMap, OutSaveData.DiplomacyActionHistory, OutSaveData.Attitudes, OutSaveData.NextActionId);
    }

    return true;
}

bool USaveLoadManager::SaveGameToSlot(int32 SlotIndex, const FString& SaveGameName)
{
    if (!IsValidSlotIndex(SlotIndex))
    {
        return false;
    }

    if (!GameInstance)
    {
        return false;
    }

    // 게임 상태 수집
    FGameSaveData SaveData;
    if (!CollectGameStateForSave(SaveData))
    {
        return false;
    }

    // 세이브 데이터 설정
    SaveData.SaveGameName = SaveGameName.IsEmpty() ? FString::Printf(TEXT("Save_%d"), SlotIndex) : SaveGameName;
    SaveData.SaveSlotIndex = SlotIndex;

    // USuperSaveGame 객체 생성
    USuperSaveGame* SaveGameObject = Cast<USuperSaveGame>(UGameplayStatics::CreateSaveGameObject(USuperSaveGame::StaticClass()));
    if (!SaveGameObject)
    {
        return false;
    }

    // 세이브 데이터 저장
    SaveGameObject->SaveData = SaveData;

    // 파일로 저장
    FString SlotName = GetSaveSlotName(SlotIndex);
    return UGameplayStatics::SaveGameToSlot(SaveGameObject, SlotName, 0);
}

// ========== 로드 함수 ==========

bool USaveLoadManager::LoadGameFromSlot(int32 SlotIndex)
{
    if (!IsValidSlotIndex(SlotIndex))
    {
        return false;
    }

    if (!GameInstance)
    {
        return false;
    }

    // 세이브 파일 로드
    FString SlotName = GetSaveSlotName(SlotIndex);
    USuperSaveGame* SaveGameObject = Cast<USuperSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!SaveGameObject)
    {
        return false;
    }

    // 게임 상태 복원
    return RestoreGameStateFromSave(SaveGameObject->SaveData);
}

bool USaveLoadManager::RestoreGameStateFromSave(const FGameSaveData& SaveData)
{
    if (!GameInstance)
    {
        return false;
    }

    UWorld* World = GameInstance->GetWorld();
    if (!World)
    {
        return false;
    }

    // 라운드 정보 복원 (세이브는 항상 플레이어 턴에서만 하므로, 로드 시에도 플레이어 턴으로 시작)
    ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(World->GetAuthGameMode());
    if (GameMode && GameMode->GetTurnComponent())
    {
        // InitializeTurn은 라운드 번호만 받아서 TurnNumber=1, PlayerIndex=0으로 초기화
        GameMode->GetTurnComponent()->InitializeTurn(SaveData.CurrentRound);
    }

    // CountryNames 배열 복원 (메인메뉴에서 로드 시 필요)
    // PlayerDataArray를 순회하면서 각 플레이어의 CountryRowName을 수집
    TArray<FName> RestoredCountryNames;
    for (const FPlayerSaveData& PlayerData : SaveData.PlayerDataArray)
    {
        // PlayerIndex 순서대로 CountryRowName 추가
        // PlayerIndex가 배열 인덱스와 일치하도록 정렬
        int32 PlayerIndex = PlayerData.PlayerIndex;
        while (RestoredCountryNames.Num() <= PlayerIndex)
        {
            RestoredCountryNames.Add(NAME_None);
        }
        if (PlayerIndex >= 0 && PlayerIndex < RestoredCountryNames.Num())
        {
            RestoredCountryNames[PlayerIndex] = PlayerData.CountryRowName;
        }
    }
    // GameInstance에 CountryNames 설정
    GameInstance->SetCountryNames(RestoredCountryNames);

    // 플레이어 데이터 복원 (도시 데이터 포함)
    for (const FPlayerSaveData& PlayerData : SaveData.PlayerDataArray)
    {
        ASuperPlayerState* PlayerState = GameInstance->GetPlayerState(PlayerData.PlayerIndex);
        if (PlayerState)
        {
            RestorePlayerData(PlayerData, PlayerState);
        }
    }

    // 타일 소유권은 RestorePlayerData에서 OwnedTileCoordinates를 통해 복원되므로 별도로 복원하지 않음

    // 월드 타일 데이터 복원 (지형, 기후, 자원, 숲, 시설 정보)
    if (GameInstance->GetGeneratedWorldComponent() && GameInstance->GetFacilityManager())
    {
        RestoreWorldData(SaveData.WorldDataMap, GameInstance->GetGeneratedWorldComponent(), GameInstance->GetFacilityManager());
    }

    // 외교 데이터 복원
    if (GameInstance->GetDiplomacyManager())
    {
        RestoreDiplomacyData(SaveData.DiplomacyStateMap, SaveData.DiplomacyActionHistory, SaveData.Attitudes, SaveData.NextActionId, GameInstance->GetDiplomacyManager());
    }

    // 유닛 데이터 복원 (마지막에 - 스폰이 필요하므로)
    // 각 플레이어의 유닛 데이터를 복원
    if (GameInstance->GetUnitManager())
    {
        for (const FPlayerSaveData& PlayerData : SaveData.PlayerDataArray)
        {
            for (const FUnitSaveData& UnitData : PlayerData.UnitDataArray)
            {
                RestoreUnitData(UnitData, PlayerData.PlayerIndex);
            }
        }
    }

    return true;
}

// ========== 세이브 슬롯 관리 ==========

bool USaveLoadManager::DoesSaveGameExist(int32 SlotIndex)
{
    if (!IsValidSlotIndex(SlotIndex))
    {
        return false;
    }

    FString SlotName = GetSaveSlotName(SlotIndex);
    return UGameplayStatics::DoesSaveGameExist(SlotName, 0);
}

bool USaveLoadManager::GetSaveGameInfo(int32 SlotIndex, FString& OutSaveGameName, FDateTime& OutSaveDateTime)
{
    if (!IsValidSlotIndex(SlotIndex))
    {
        return false;
    }

    FString SlotName = GetSaveSlotName(SlotIndex);
    USuperSaveGame* SaveGameObject = Cast<USuperSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));

    if (!SaveGameObject)
    {
        return false;
    }

    // SaveData에서 메타 정보 가져오기
    OutSaveGameName = SaveGameObject->SaveData.SaveGameName;
    OutSaveDateTime = SaveGameObject->SaveData.SaveDateTime;
    return true;
}

bool USaveLoadManager::DeleteSaveGame(int32 SlotIndex)
{
    if (!IsValidSlotIndex(SlotIndex))
    {
        return false;
    }

    FString SlotName = GetSaveSlotName(SlotIndex);
    return UGameplayStatics::DeleteGameInSlot(SlotName, 0);
}

TArray<FSaveSlotInfo> USaveLoadManager::GetAllSaveSlotInfos()
{
    TArray<FSaveSlotInfo> SlotInfos;

    for (int32 i = MIN_SAVE_SLOT; i <= MAX_SAVE_SLOT; i++)
    {
        FSaveSlotInfo SlotInfo;
        SlotInfo.SlotIndex = i;
        SlotInfo.bIsValid = DoesSaveGameExist(i);
        SlotInfos.Add(SlotInfo);
    }

    return SlotInfos;
}

// ========== 데이터 수집 헬퍼 함수들 ==========

void USaveLoadManager::CollectPlayerData(ASuperPlayerState* PlayerState, FPlayerSaveData& OutPlayerData)
{
    if (!PlayerState)
    {
        return;
    }

    OutPlayerData.PlayerIndex = PlayerState->PlayerIndex;
    OutPlayerData.CountryRowName = PlayerState->CountryRowName;
    // CountryName과 BorderColor는 CountryRowName을 통해 데이터 테이블에서 조회 가능하므로 저장하지 않음

    // 자원
    OutPlayerData.Food = PlayerState->Food;
    OutPlayerData.Production = PlayerState->Production;
    OutPlayerData.Gold = PlayerState->Gold;
    OutPlayerData.Science = PlayerState->Science;
    OutPlayerData.Faith = PlayerState->Faith;

    // 인구
    OutPlayerData.Population = PlayerState->Population;
    OutPlayerData.LimitPopulation = PlayerState->LimitPopulation;

    // 타일 소유권
    OutPlayerData.OwnedTileCoordinates = PlayerState->GetOwnedTileCoordinates();

    // 도시 정보
    OutPlayerData.bHasCity = PlayerState->HasCity();
    if (OutPlayerData.bHasCity && PlayerState->GetCityComponent())
    {
        CollectCityData(PlayerState->GetCityComponent(), OutPlayerData.CityData);
        OutPlayerData.CityData.CityCoordinate = PlayerState->GetCityCoordinate();
    }

    // 자원 보유
    OutPlayerData.OwnedLuxuryResources = PlayerState->GetOwnedLuxuryResources();
    OutPlayerData.OwnedStrategicResources = PlayerState->GetOwnedStrategicResources();
    OutPlayerData.OwnedStrategicResourceStocks = PlayerState->OwnedStrategicResourceStocks;

    // 시설 건설 가능 목록
    OutPlayerData.AvailableFacilities = PlayerState->GetAvailableFacilities();

    // 연구 데이터
    if (PlayerState->GetResearchComponent())
    {
        FResearchData ResearchData = PlayerState->GetResearchComponent()->GetResearchData();
        FResearchCurrentStat CurrentStat = PlayerState->GetResearchComponent()->GetCurrentStat();

        OutPlayerData.ResearchData.ResearchedTechs = ResearchData.ResearchedTechs;
        OutPlayerData.ResearchData.DevelopingName = CurrentStat.DevelopingName;
        OutPlayerData.ResearchData.DevelopingProgress = CurrentStat.DevelopingProgress;
        // DevelopingCost는 DevelopingName을 통해 데이터 테이블에서 조회 가능하므로 저장하지 않음
    }

    // 패배 상태 저장
    OutPlayerData.bIsDefeated = PlayerState->bIsDefeated;
}

void USaveLoadManager::CollectCityData(UCityComponent* CityComponent, FCitySaveData& OutCityData)
{
    if (!CityComponent)
    {
        return;
    }

    // PlayerIndex는 FPlayerSaveData에서 관리하므로 FCitySaveData에는 저장하지 않음
    // CityName은 런타임에 설정되는 값이므로 저장하지 않음
    OutCityData.RemainingHealth = CityComponent->GetCurrentHealth();

    // 건물 목록
    OutCityData.BuiltBuildings = CityComponent->GetBuiltBuildings();

    // 생산 정보
    FCityCurrentStat CurrentStat = CityComponent->GetCurrentStat();
    OutCityData.ProductionType = CurrentStat.ProductionType;
    OutCityData.ProductionName = CurrentStat.ProductionName;
    OutCityData.ProductionProgress = CurrentStat.ProductionProgress;
    // ProductionCost는 ProductionType과 ProductionName을 통해 데이터 테이블에서 조회 가능하므로 저장하지 않음
    OutCityData.FoodProgress = CurrentStat.FoodProgress;
    // FoodCost는 ProductionName(Unit)을 통해 데이터 테이블에서 조회 가능하므로 저장하지 않음

    // 도시 좌표는 PlayerState에서 가져와야 함
    // (CityComponent에는 좌표 정보가 없음)
}

void USaveLoadManager::CollectUnitData(AUnitCharacterBase* Unit, FUnitSaveData& OutUnitData)
{
    if (!Unit)
    {
        return;
    }

    OutUnitData.UnitDataRowName = Unit->GetUnitDataRowName();
    // PlayerIndex는 부모 FPlayerSaveData의 PlayerIndex를 사용하므로 저장하지 않음

    // 그리드 좌표는 UnitManager에서 정확한 Hex 위치 가져오기
    if (GameInstance && GameInstance->GetUnitManager())
    {
        FVector2D GridPos = GameInstance->GetUnitManager()->GetHexPositionForUnit(Unit);
        OutUnitData.GridPosition = GridPos;
        
        // UnitManager에서 찾지 못한 경우에만 WorldToHex 사용 (폴백)
        if (GridPos == FVector2D::ZeroVector && GameInstance->GetGeneratedWorldComponent())
        {
            FVector WorldPos = Unit->GetActorLocation();
            GridPos = GameInstance->GetGeneratedWorldComponent()->WorldToHex(WorldPos);
            // 소수점 좌표를 정수로 반올림
            GridPos.X = FMath::RoundToInt(GridPos.X);
            GridPos.Y = FMath::RoundToInt(GridPos.Y);
            OutUnitData.GridPosition = GridPos;
        }
    }

    // 현재 상태
    if (Unit->GetUnitStatusComponent())
    {
        FUnitCurrentStat CurrentStat = Unit->GetUnitStatusComponent()->GetCurrentStat();
        OutUnitData.RemainingHealth = CurrentStat.RemainingHealth;
        OutUnitData.RemainingMovementPoints = CurrentStat.RemainingMovementPoints;
        OutUnitData.HasAttacked = CurrentStat.HasAttacked;
        OutUnitData.IsWait = CurrentStat.IsWait;
        OutUnitData.IsAlert = CurrentStat.IsAlert;
        OutUnitData.IsSleep = CurrentStat.IsSleep;
    }
}

void USaveLoadManager::CollectWorldData(UWorldComponent* WorldComponent, UFacilityManager* FacilityManager, TMap<FVector2D, FWorldSaveData>& OutWorldDataMap)
{
    if (!WorldComponent)
    {
        return;
    }

    OutWorldDataMap.Empty();

    // 모든 타일을 순회하며 데이터 수집
    TArray<UWorldTile*> AllTiles = WorldComponent->GetAllTiles();
    for (UWorldTile* Tile : AllTiles)
    {
        if (!Tile)
        {
            continue;
        }

        FWorldSaveData WorldData;
        FVector2D GridPos = Tile->GetGridPosition();
        // GridPosition은 TMap의 키로 사용되므로 저장하지 않음

        // 지형 정보
        WorldData.TerrainType = Tile->GetTerrainType();
        WorldData.ClimateType = Tile->GetClimateType();
        WorldData.LandType = Tile->GetLandType();
        WorldData.bHasForest = Tile->HasForest();

        // 자원 정보
        WorldData.ResourceCategory = Tile->GetResourceCategory();
        WorldData.BonusResource = Tile->GetBonusResource();
        WorldData.StrategicResource = Tile->GetStrategicResource();
        WorldData.LuxuryResource = Tile->GetLuxuryResource();

        // 시설 정보
        WorldData.FacilityRowName = NAME_None;
        WorldData.bIsPillaged = false;

        if (FacilityManager && FacilityManager->HasFacilityAtTile(GridPos))
        {
            // FacilityManager의 BuiltFacilities에서 시설 정보 가져오기
            if (FacilityManager->FacilityDataTable && FacilityManager->BuiltFacilities.Contains(GridPos))
            {
                FFacilityData FacilityData = FacilityManager->BuiltFacilities[GridPos];
                WorldData.bIsPillaged = FacilityData.bIsPillaged;

                // 시설 데이터 테이블에서 FacilityName으로 RowName 찾기
                FString TargetFacilityName = FacilityData.FacilityName;
                TArray<FName> RowNames = FacilityManager->FacilityDataTable->GetRowNames();
                for (const FName& RowName : RowNames)
                {
                    FFacilityData* FacilityDataPtr = FacilityManager->FacilityDataTable->FindRow<FFacilityData>(RowName, TEXT(""));
                    if (FacilityDataPtr && FacilityDataPtr->FacilityName == TargetFacilityName)
                    {
                        WorldData.FacilityRowName = RowName;
                        break;
                    }
                }
            }
        }

        OutWorldDataMap.Add(GridPos, WorldData);
    }
}

void USaveLoadManager::CollectDiplomacyData(UDiplomacyManager* DiplomacyManager, TMap<FDiplomacyPairKey, FDiplomacyPairState>& OutDiplomacyStateMap, TArray<FDiplomacyAction>& OutDiplomacyActionHistory, TMap<FAttitudeKey, int32>& OutAttitudes, int32& OutNextActionId)
{
    if (!DiplomacyManager)
    {
        return;
    }

    // 외교 상태 맵 복사
    OutDiplomacyStateMap = DiplomacyManager->PairStates;

    // 외교 액션 히스토리 복사
    OutDiplomacyActionHistory = DiplomacyManager->PendingActions;

    // 호감도 맵 변환 (TMap<int32, TMap<int32, int32>> -> TMap<FAttitudeKey, int32>)
    OutAttitudes.Empty();
    for (const auto& OuterPair : DiplomacyManager->Attitudes)
    {
        int32 FromPlayerIndex = OuterPair.Key;
        for (const auto& InnerPair : OuterPair.Value)
        {
            int32 ToPlayerIndex = InnerPair.Key;
            int32 Attitude = InnerPair.Value;
            FAttitudeKey Key(FromPlayerIndex, ToPlayerIndex);
            OutAttitudes.Add(Key, Attitude);
        }
    }

    // 다음 액션 ID 복사
    OutNextActionId = DiplomacyManager->NextActionId;
}

// ========== 데이터 복원 헬퍼 함수들 ==========

void USaveLoadManager::RestorePlayerData(const FPlayerSaveData& PlayerData, ASuperPlayerState* PlayerState)
{
    if (!PlayerState)
    {
        return;
    }

    // 국가 정보
    PlayerState->CountryRowName = PlayerData.CountryRowName;
    // CountryName과 BorderColor는 데이터 테이블에서 로드 (LoadCountryDataFromTable 호출 필요)
    PlayerState->LoadCountryDataFromTable();

    // 자원
    PlayerState->Food = PlayerData.Food;
    PlayerState->Production = PlayerData.Production;
    PlayerState->Gold = PlayerData.Gold;
    PlayerState->Science = PlayerData.Science;
    PlayerState->Faith = PlayerData.Faith;

    // 인구
    PlayerState->Population = PlayerData.Population;
    PlayerState->LimitPopulation = PlayerData.LimitPopulation;

    // 타일 소유권 복원
    if (GameInstance && GameInstance->GetGeneratedWorldComponent())
    {
        PlayerState->ClearAllOwnedTiles(GameInstance->GetGeneratedWorldComponent());
        for (const FVector2D& TileCoord : PlayerData.OwnedTileCoordinates)
        {
            PlayerState->AddOwnedTile(TileCoord, GameInstance->GetGeneratedWorldComponent());
        }
    }

    // 도시 데이터 복원
    if (PlayerData.bHasCity)
    {
        RestoreCityData(PlayerData.CityData, PlayerState);
    }

    // 자원 보유
    PlayerState->OwnedLuxuryResources = PlayerData.OwnedLuxuryResources;
    PlayerState->OwnedStrategicResources = PlayerData.OwnedStrategicResources;
    PlayerState->OwnedStrategicResourceStocks = PlayerData.OwnedStrategicResourceStocks;

    // 시설 건설 가능 목록
    PlayerState->AvailableFacilities = PlayerData.AvailableFacilities;

    // 연구 데이터 복원
    if (PlayerState->GetResearchComponent())
    {
        // 연구 완료된 기술 목록 복원
        FResearchData ResearchData;
        ResearchData.ResearchedTechs = PlayerData.ResearchData.ResearchedTechs;
        PlayerState->GetResearchComponent()->InitFromResearchData(ResearchData);

        // 현재 연구 중인 기술 복원
        if (PlayerData.ResearchData.DevelopingName != NAME_None)
        {
            PlayerState->StartTechResearch(PlayerData.ResearchData.DevelopingName);
            // 연구 진행도 복원
            PlayerState->GetResearchComponent()->SetResearchProgress(PlayerData.ResearchData.DevelopingProgress);
        }
    }

    // 패배 상태 복원 (단순 대입, SetDefeated() 호출하지 않음!)
    // SetDefeated()를 호출하면 CleanupDefeatedPlayer()가 다시 실행되어 문제 발생!
    PlayerState->bIsDefeated = PlayerData.bIsDefeated;
}

void USaveLoadManager::RestoreCityData(const FCitySaveData& CityData, ASuperPlayerState* PlayerState)
{
    if (!PlayerState)
    {
        return;
    }

    // 도시 컴포넌트 확인
    if (!PlayerState->GetCityComponent())
    {
        return;
    }

    UCityComponent* CityComponent = PlayerState->GetCityComponent();

    // 도시 이름은 런타임에 설정되는 값이므로 복원하지 않음

    // 체력 복원
    int32 CurrentHealth = CityComponent->GetCurrentHealth();
    int32 HealthDiff = CityData.RemainingHealth - CurrentHealth;
    if (HealthDiff > 0)
    {
        CityComponent->Heal(HealthDiff);
    }
    else if (HealthDiff < 0)
    {
        CityComponent->TakeDamage(-HealthDiff);
    }

    // 건물 복원
    for (const FName& BuildingName : CityData.BuiltBuildings)
    {
        CityComponent->AddBuilding(BuildingName);
    }

    // 생산 정보 복원
    if (CityData.ProductionType == EProductionType::Building)
    {
        CityComponent->StartBuildingProduction(CityData.ProductionName);
        // 생산 진행도 복원
        CityComponent->SetProductionProgress(CityData.ProductionProgress);
    }
    else if (CityData.ProductionType == EProductionType::Unit)
    {
        CityComponent->StartUnitProduction(CityData.ProductionName);
        // 식량 진행도 복원
        CityComponent->SetFoodProgress(CityData.FoodProgress);
    }

    // 도시 좌표 설정
    PlayerState->SetCityCoordinate(CityData.CityCoordinate);

    // 로드 후 스모그·도시 머리 위 UI 갱신 (체력 비율에 맞게)
    if (UWorld* World = PlayerState->GetWorld())
    {
        if (AWorldSpawner* WorldSpawner = Cast<AWorldSpawner>(UGameplayStatics::GetActorOfClass(World, AWorldSpawner::StaticClass())))
        {
            const FVector2D RoundedHex(FMath::RoundToInt(CityData.CityCoordinate.X), FMath::RoundToInt(CityData.CityCoordinate.Y));
            if (ACityActor* CityActor = WorldSpawner->GetCityActorAtHex(RoundedHex))
            {
                const int32 CurrentHP = CityComponent->GetCurrentHealth();
                const int32 MaxHP = CityComponent->GetMaxHealth();
                CityActor->UpdateCitySmogVisibility(CurrentHP, MaxHP);
                CityActor->UpdateSmallCityUI(PlayerState->GetCountryName(), PlayerState->GetCountryLargeImg(), CurrentHP, MaxHP);
            }
        }
    }
    // bHasCity는 이미 true여야 함 (도시가 있어야 CityComponent가 있음)
}

void USaveLoadManager::RestoreUnitData(const FUnitSaveData& UnitData, int32 PlayerIndex)
{
    if (!GameInstance || !GameInstance->GetUnitManager())
    {
        return;
    }

    // GridPosition을 정수로 반올림 (소수점 좌표 문제 해결)
    FVector2D RoundedGridPosition;
    RoundedGridPosition.X = FMath::RoundToInt(UnitData.GridPosition.X);
    RoundedGridPosition.Y = FMath::RoundToInt(UnitData.GridPosition.Y);

    // 유닛 스폰 (로드 시에는 배치 체크 건너뛰기)
    AUnitCharacterBase* Unit = GameInstance->GetUnitManager()->SpawnUnitAtHex(
        RoundedGridPosition,
        UnitData.UnitDataRowName,
        PlayerIndex,
        true // bSkipPlacementCheck = true (로드 시에는 배치 체크 건너뛰기)
    );

    if (!Unit)
    {
        return;
    }

    // 현재 상태 복원
    if (Unit->GetUnitStatusComponent())
    {
        // 체력 복원
        int32 CurrentHealth = Unit->GetUnitStatusComponent()->GetCurrentHealth();
        int32 HealthDiff = UnitData.RemainingHealth - CurrentHealth;
        if (HealthDiff > 0)
        {
            Unit->GetUnitStatusComponent()->Heal(HealthDiff);
        }
        else if (HealthDiff < 0)
        {
            Unit->GetUnitStatusComponent()->TakeDamage(-HealthDiff);
        }

        // 이동력 복원
        int32 MovementDiff = UnitData.RemainingMovementPoints - Unit->GetUnitStatusComponent()->GetCurrentStat().RemainingMovementPoints;
        if (MovementDiff != 0)
        {
            // 이동력은 직접 설정 불가능하므로, ConsumeMovement로 조정
            if (MovementDiff < 0)
            {
                Unit->GetUnitStatusComponent()->ConsumeMovement(-MovementDiff);
            }
        }

        // 턴 상태 복원
        Unit->GetUnitStatusComponent()->SetHasAttacked(UnitData.HasAttacked);
        Unit->GetUnitStatusComponent()->SetWait(UnitData.IsWait);
        Unit->GetUnitStatusComponent()->SetAlert(UnitData.IsAlert);
        Unit->GetUnitStatusComponent()->SetSleep(UnitData.IsSleep);
    }

    // 머리 위 HP 바 갱신 (스폰 시 1.0으로 설정된 뒤 체력을 복원했으므로, UI를 현재 체력에 맞게 갱신)
    if (USmallUnitUI* SmallUI = Unit->GetSmallUnitUI())
    {
        if (UUnitStatusComponent* StatusComp = Unit->GetUnitStatusComponent())
        {
            int32 CurrentHP = StatusComp->GetCurrentHealth();
            int32 MaxHP = StatusComp->GetMaxHealth();
            float HPPercent = (MaxHP > 0) ? FMath::Clamp((float)CurrentHP / (float)MaxHP, 0.0f, 1.0f) : 1.0f;
            SmallUI->SetHPBar(HPPercent);
        }
    }

    // 위치는 SpawnUnitAtHex()에서 이미 정확하게 설정되므로 추가 설정 불필요
}

void USaveLoadManager::RestoreWorldData(const TMap<FVector2D, FWorldSaveData>& WorldDataMap, UWorldComponent* WorldComponent, UFacilityManager* FacilityManager)
{
    if (!WorldComponent)
    {
        return;
    }

    // WorldSpawner 찾기 (타일 액터 시각 업데이트용)
    AWorldSpawner* WorldSpawner = nullptr;
    if (GameInstance && GameInstance->GetWorld())
    {
        for (TActorIterator<AWorldSpawner> ActorItr(GameInstance->GetWorld()); ActorItr; ++ActorItr)
        {
            WorldSpawner = *ActorItr;
            break;
        }
    }

    // 메인메뉴에서 로드한 경우: 월드는 이미 GenerateWorldFromSaveData()로 생성되었으므로
    // 시설 정보만 복원하면 됩니다.
    // 게임 중 로드한 경우: 타일 데이터도 복원해야 합니다.
    bool bWorldAlreadyGeneratedFromSave = GameInstance && GameInstance->bIsLoadingFromMainMenu;

    for (const auto& Pair : WorldDataMap)
    {
        UWorldTile* Tile = WorldComponent->GetTileAtHex(Pair.Key);
        if (!Tile)
        {
            continue;
        }

        const FWorldSaveData& WorldData = Pair.Value;

        // 월드가 이미 저장된 데이터로 생성되지 않은 경우에만 타일 데이터 복원
        if (!bWorldAlreadyGeneratedFromSave)
        {
            // 지형 정보 복원 (데이터 변경)
            Tile->SetTerrainType(WorldData.TerrainType);
            Tile->SetClimateType(WorldData.ClimateType);
            Tile->SetLandType(WorldData.LandType);
            Tile->SetHasForest(WorldData.bHasForest);

            // 자원 정보 복원 (데이터 변경)
            Tile->SetResourceCategory(WorldData.ResourceCategory);
            Tile->SetBonusResource(WorldData.BonusResource);
            Tile->SetStrategicResource(WorldData.StrategicResource);
            Tile->SetLuxuryResource(WorldData.LuxuryResource);

            // 타일 생산량 재계산 (데이터 변경 후 필수)
            WorldComponent->RecalculateTileYields(Tile);

            // 타일 액터 시각 업데이트 (이미 스폰된 경우)
            if (WorldSpawner)
            {
                WorldSpawner->UpdateTileVisual(Pair.Key);
            }
        }

        // 시설 정보 복원 (항상 수행)
        if (WorldData.FacilityRowName != NAME_None && FacilityManager && GameInstance && GameInstance->GetGeneratedWorldComponent())
        {
            // 시설 건설 (FacilityManager의 BuildFacility 함수 사용)
            FacilityManager->BuildFacility(WorldData.FacilityRowName, Pair.Key, GameInstance->GetGeneratedWorldComponent());

            // 약탈 상태 설정
            if (WorldData.bIsPillaged)
            {
                FacilityManager->SetFacilityPillaged(Pair.Key, true, GameInstance->GetGeneratedWorldComponent());
            }
        }
    }
}

void USaveLoadManager::RestoreDiplomacyData(const TMap<FDiplomacyPairKey, FDiplomacyPairState>& DiplomacyStateMap, const TArray<FDiplomacyAction>& DiplomacyActionHistory, const TMap<FAttitudeKey, int32>& Attitudes, int32 NextActionId, UDiplomacyManager* DiplomacyManager)
{
    if (!DiplomacyManager)
    {
        return;
    }

    // 외교 상태 맵 복원
    DiplomacyManager->PairStates = DiplomacyStateMap;

    // 외교 액션 히스토리 복원
    DiplomacyManager->PendingActions = DiplomacyActionHistory;

    // 호감도 맵 변환 (TMap<FAttitudeKey, int32> -> TMap<int32, TMap<int32, int32>>)
    DiplomacyManager->Attitudes.Empty();
    for (const auto& Pair : Attitudes)
    {
        int32 FromPlayerIndex = Pair.Key.FromPlayerIndex;
        int32 ToPlayerIndex = Pair.Key.ToPlayerIndex;
        int32 Attitude = Pair.Value;
        
        if (!DiplomacyManager->Attitudes.Contains(FromPlayerIndex))
        {
            DiplomacyManager->Attitudes.Add(FromPlayerIndex, TMap<int32, int32>());
        }
        DiplomacyManager->Attitudes[FromPlayerIndex].Add(ToPlayerIndex, Attitude);
    }

    // 다음 액션 ID 복원 (ActionId 중복 방지)
    DiplomacyManager->NextActionId = NextActionId;

    // 라운드 정보 복원 (CachedCurrentRound, LastProcessedRound)
    // CurrentTurn은 RestoreGameStateFromSave에서 복원되므로, 여기서는 TurnComponent에서 라운드 번호를 가져옴
    if (GameInstance && GameInstance->GetWorld())
    {
        ASuperGameModeBase* GameMode = Cast<ASuperGameModeBase>(GameInstance->GetWorld()->GetAuthGameMode());
        if (GameMode && GameMode->GetTurnComponent())
        {
            int32 CurrentRound = GameMode->GetTurnComponent()->GetCurrentRoundNumber();
            DiplomacyManager->CachedCurrentRound = CurrentRound;
            // LastProcessedRound는 현재 라운드보다 작게 설정하여 OnRoundStarted가 호출될 수 있도록 함
            // (현재 라운드가 이미 처리되었는지 여부는 게임 로직에 따라 다를 수 있으므로, 안전하게 CurrentRound - 1로 설정)
            DiplomacyManager->LastProcessedRound = CurrentRound - 1;
        }
    }
}

// ========== 내부 헬퍼 함수들 ==========

FString USaveLoadManager::GetSaveSlotName(int32 SlotIndex) const
{
    return FString::Printf(TEXT("SaveSlot%d"), SlotIndex);
}

bool USaveLoadManager::IsValidSlotIndex(int32 SlotIndex) const
{
    return SlotIndex >= MIN_SAVE_SLOT && SlotIndex <= MAX_SAVE_SLOT;
}

