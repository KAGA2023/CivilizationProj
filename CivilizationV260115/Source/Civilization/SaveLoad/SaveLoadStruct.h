// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameFramework/SaveGame.h"
#include "../World/WorldStruct.h"
#include "../City/CityStruct.h"
#include "../Research/Research.h"
#include "../Status/UnitStatusStruct.h"
#include "../Diplomacy/DiplomacyStruct.h"
#include "../Facility/FacilityStruct.h"
#include "SaveLoadStruct.generated.h"

// ========== 연구 세이브 데이터 ==========
USTRUCT(BlueprintType)
struct CIVILIZATION_API FResearchSaveData
{
    GENERATED_BODY()

    // 연구 완료된 기술 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    TArray<FName> ResearchedTechs;

    // 연구 중인 기술
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    FName DevelopingName = NAME_None;

    // 연구 진행도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    int32 DevelopingProgress = 0;

    FResearchSaveData()
    {
        ResearchedTechs.Empty();
        DevelopingName = NAME_None;
        DevelopingProgress = 0;
    }
};

// ========== 도시 세이브 데이터 ==========
USTRUCT(BlueprintType)
struct CIVILIZATION_API FCitySaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Info")
    FVector2D CityCoordinate = FVector2D::ZeroVector;

    // ========== 도시 상태 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Status")
    int32 RemainingHealth = 0;

    // ========== 건물 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buildings")
    TArray<FName> BuiltBuildings;

    // ========== 생산 정보 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EProductionType ProductionType = EProductionType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FName ProductionName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 ProductionProgress = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 FoodProgress = 0;

    FCitySaveData()
    {
        CityCoordinate = FVector2D::ZeroVector;
        RemainingHealth = 0;
        BuiltBuildings.Empty();
        ProductionType = EProductionType::None;
        ProductionName = NAME_None;
        ProductionProgress = 0;
        FoodProgress = 0;
    }
};

// ========== 유닛 세이브 데이터 ==========
USTRUCT(BlueprintType)
struct CIVILIZATION_API FUnitSaveData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    FName UnitDataRowName = NAME_None;

    // ========== 위치 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
    FVector2D GridPosition = FVector2D::ZeroVector;

    // ========== 현재 상태 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    int32 RemainingHealth = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    int32 RemainingMovementPoints = 0;

    // ========== 턴 상태 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Status")
    bool HasAttacked = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Status")
    bool IsWait = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Status")
    bool IsAlert = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Status")
    bool IsSleep = false;

    FUnitSaveData()
    {
        UnitDataRowName = NAME_None;
        GridPosition = FVector2D::ZeroVector;
        RemainingHealth = 0;
        RemainingMovementPoints = 0;
        HasAttacked = false;
        IsWait = false;
        IsAlert = false;
        IsSleep = false;
    }
};

// ========== 플레이어 세이브 데이터 ==========
USTRUCT(BlueprintType)
struct CIVILIZATION_API FPlayerSaveData
{
    GENERATED_BODY()

    // ========== 기본 정보 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
    int32 PlayerIndex = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
    FName CountryRowName = NAME_None;

    // ========== 자원 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Food = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Production = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Gold = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Science = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Faith = 0;

    // ========== 인구 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    int32 Population = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    int32 LimitPopulation = 4;

    // ========== 타일 소유권 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
    TArray<FVector2D> OwnedTileCoordinates;

    // ========== 도시 정보 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    bool bHasCity = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FCitySaveData CityData;

    // ========== 자원 보유 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TMap<ELuxuryResource, int32> OwnedLuxuryResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TMap<EStrategicResource, int32> OwnedStrategicResources;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TMap<EStrategicResource, int32> OwnedStrategicResourceStocks;

    // ========== 연구 데이터 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    FResearchSaveData ResearchData;

    // ========== 시설 건설 가능 목록 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility")
    TArray<FName> AvailableFacilities;

    // ========== 유닛 데이터 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Data")
    TArray<FUnitSaveData> UnitDataArray;

    FPlayerSaveData()
    {
        PlayerIndex = -1;
        CountryRowName = NAME_None;
        Food = 0;
        Production = 0;
        Gold = 0;
        Science = 0;
        Faith = 0;
        Population = 0;
        LimitPopulation = 4;
        bHasCity = false;
        OwnedLuxuryResources.Empty();
        OwnedStrategicResources.Empty();
        OwnedStrategicResourceStocks.Empty();
        AvailableFacilities.Empty();
        UnitDataArray.Empty();
    }
};

// ========== 월드 타일 세이브 데이터 ==========
USTRUCT(BlueprintType)
struct CIVILIZATION_API FWorldSaveData
{
    GENERATED_BODY()

    // ========== 지형 정보 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    ETerrainType TerrainType = ETerrainType::Land;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EClimateType ClimateType = EClimateType::Temperate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    ELandType LandType = ELandType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bHasForest = false;

    // ========== 자원 정보 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    EResourceCategory ResourceCategory = EResourceCategory::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    EBonusResource BonusResource = EBonusResource::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    EStrategicResource StrategicResource = EStrategicResource::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    ELuxuryResource LuxuryResource = ELuxuryResource::None;

    // ========== 시설 정보 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility")
    FName FacilityRowName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility")
    bool bIsPillaged = false; // 시설이 약탈되었는지 여부

    FWorldSaveData()
    {
        TerrainType = ETerrainType::Land;
        ClimateType = EClimateType::Temperate;
        LandType = ELandType::Plains;
        bHasForest = false;
        ResourceCategory = EResourceCategory::None;
        BonusResource = EBonusResource::None;
        StrategicResource = EStrategicResource::None;
        LuxuryResource = ELuxuryResource::None;
        FacilityRowName = NAME_None;
        bIsPillaged = false;
    }
};

// ========== 세이브 슬롯 정보 ==========
USTRUCT(BlueprintType)
struct CIVILIZATION_API FSaveSlotInfo
{
    GENERATED_BODY()

    // 슬롯 인덱스 (1~5)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Slot")
    int32 SlotIndex = 0;

    // 유효한 세이브 파일이 존재하는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Slot")
    bool bIsValid = false;

    FSaveSlotInfo()
    {
        SlotIndex = 0;
        bIsValid = false;
    }
};

// ========== 호감도 키 (평면화된 구조) ==========
USTRUCT(BlueprintType)
struct CIVILIZATION_API FAttitudeKey
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attitude")
    int32 FromPlayerIndex = -1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attitude")
    int32 ToPlayerIndex = -1;

    FAttitudeKey()
    {
        FromPlayerIndex = -1;
        ToPlayerIndex = -1;
    }

    FAttitudeKey(int32 From, int32 To)
    {
        FromPlayerIndex = From;
        ToPlayerIndex = To;
    }

    bool operator==(const FAttitudeKey& Other) const
    {
        return FromPlayerIndex == Other.FromPlayerIndex && ToPlayerIndex == Other.ToPlayerIndex;
    }

    friend uint32 GetTypeHash(const FAttitudeKey& Key)
    {
        return HashCombine(GetTypeHash(Key.FromPlayerIndex), GetTypeHash(Key.ToPlayerIndex));
    }
};

// ========== 게임 세이브 데이터 (최상위) ==========
USTRUCT(BlueprintType)
struct CIVILIZATION_API FGameSaveData
{
    GENERATED_BODY()

    // ========== 게임 메타 정보 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Info")
    FString SaveGameName = TEXT("");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Info")
    FDateTime SaveDateTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Info")
    int32 SaveSlotIndex = 0;

    // ========== 게임 진행 상태 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    int32 CurrentRound = 1;
    // 세이브는 항상 플레이어 턴(TurnNumber=1, PlayerIndex=0)에서만 하므로, 라운드 번호만 저장

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    FWorldConfig WorldConfig;
    // 메인메뉴에서 로드 시 월드를 생성하기 위해 WorldConfig 저장 필요

    // ========== 플레이어 데이터 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Data")
    TArray<FPlayerSaveData> PlayerDataArray;

    // ========== 월드 타일 데이터 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Data")
    TMap<FVector2D, FWorldSaveData> WorldDataMap;

    // ========== 외교 데이터 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy Data")
    TMap<FDiplomacyPairKey, FDiplomacyPairState> DiplomacyStateMap;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy Data")
    TArray<FDiplomacyAction> DiplomacyActionHistory;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy Data")
    TMap<FAttitudeKey, int32> Attitudes; // 플레이어 간 호감도 맵 (평면화된 구조: FromPlayerIndex -> ToPlayerIndex -> 호감도)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy Data")
    int32 NextActionId = 1; // 다음 외교 액션 ID (ActionId 중복 방지)

    FGameSaveData()
    {
        SaveGameName = TEXT("");
        SaveDateTime = FDateTime::Now();
        SaveSlotIndex = 0;
        CurrentRound = 1;
        WorldConfig = FWorldConfig();
        PlayerDataArray.Empty();
        WorldDataMap.Empty();
        DiplomacyStateMap.Empty();
        DiplomacyActionHistory.Empty();
        Attitudes.Empty();
        NextActionId = 1;
    }
};

// ========== USaveGame 상속 클래스 ==========
UCLASS()
class CIVILIZATION_API USuperSaveGame : public USaveGame
{
    GENERATED_BODY()

public:
    UPROPERTY(VisibleAnywhere, Category = "Save Data")
    FGameSaveData SaveData;

    USuperSaveGame()
    {
    }
};

