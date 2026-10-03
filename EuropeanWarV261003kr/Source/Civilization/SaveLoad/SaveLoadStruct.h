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

    // 이미 끝난 기술 RowName
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    TArray<FName> ResearchedTechs;

    // 지금 연구 중인 기술
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

    // 도시 hex
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Info")
    FVector2D CityCoordinate = FVector2D::ZeroVector;

    // ========== 도시 상태 ==========

    // 남은 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Status")
    int32 RemainingHealth = 0;

    // ========== 건물 ==========

    // 이미 지은 건물 RowName
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buildings")
    TArray<FName> BuiltBuildings;

    // ========== 생산 정보 ==========

    // 지금 생산 중인 대상
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EProductionType ProductionType = EProductionType::None;

    // 생산 중인 건물 또는 유닛
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FName ProductionName = NAME_None;

    // 건물 생산 진행도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 ProductionProgress = 0;

    // 유닛 생산 식량 진행도
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

    // 유닛 데이터 RowName
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    FName UnitDataRowName = NAME_None;

    // ========== 위치 ==========

    // 유닛 hex
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
    FVector2D GridPosition = FVector2D::ZeroVector;

    // ========== 현재 상태 ==========

    // 남은 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    int32 RemainingHealth = 0;

    // 남은 이동력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Status")
    int32 RemainingMovementPoints = 0;

    // ========== 턴 상태 ==========

    // 이번 턴에 이미 공격했는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Status")
    bool HasAttacked = false;

    // 대기 상태인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Status")
    bool IsWait = false;

    // 경계 상태인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn Status")
    bool IsAlert = false;

    // 휴면 상태인지 여부
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

    // 문명 슬롯. 0=호스트/싱글, 1=1v1 참가자, 나머지는 AI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
    int32 PlayerIndex = -1;

    // 국가 데이터 테이블 RowName
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Info")
    FName CountryRowName = NAME_None;

    // ========== 자원 ==========

    // 보유 식량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Food = 0;

    // 보유 생산력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Production = 0;

    // 보유 골드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Gold = 0;

    // 보유 과학
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Science = 0;

    // 보유 신앙
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Faith = 0;

    // ========== 인구 ==========

    // 현재 인구
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    int32 Population = 0;

    // 인구 상한
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population")
    int32 LimitPopulation = 4;

    // ========== 타일 소유권 ==========

    // 소유 타일 hex 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tiles")
    TArray<FVector2D> OwnedTileCoordinates;

    // ========== 도시 정보 ==========

    // 도시가 있는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    bool bHasCity = false;

    // 도시 세이브 데이터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City")
    FCitySaveData CityData;

    // ========== 자원 보유 ==========

    // 보유 사치 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TMap<ELuxuryResource, int32> OwnedLuxuryResources;

    // 보유 전략 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TMap<EStrategicResource, int32> OwnedStrategicResources;

    // 전략 자원 비축량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    TMap<EStrategicResource, int32> OwnedStrategicResourceStocks;

    // ========== 연구 데이터 ==========

    // 연구 완료·진행 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    FResearchSaveData ResearchData;

    // ========== 시설 건설 가능 목록 ==========

    // 지금 지을 수 있는 시설
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility")
    TArray<FName> AvailableFacilities;

    // ========== 유닛 데이터 ==========

    // 이 슬롯 유닛 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Data")
    TArray<FUnitSaveData> UnitDataArray;

    // ========== 승리/패배 상태 ==========

    // 패배했는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Victory")
    bool bIsDefeated = false;

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
        bIsDefeated = false;
    }
};

// ========== 월드 타일 세이브 데이터 ==========
USTRUCT(BlueprintType)
struct CIVILIZATION_API FWorldSaveData
{
    GENERATED_BODY()

    // ========== 지형 정보 ==========

    // 땅/바다
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    ETerrainType TerrainType = ETerrainType::Land;

    // 기후대
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    EClimateType ClimateType = EClimateType::Temperate;

    // 땅 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    ELandType LandType = ELandType::Plains;

    // 숲이 있는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain")
    bool bHasForest = false;

    // ========== 자원 정보 ==========

    // 자원 카테고리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    EResourceCategory ResourceCategory = EResourceCategory::None;

    // 보너스 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    EBonusResource BonusResource = EBonusResource::None;

    // 전략 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    EStrategicResource StrategicResource = EStrategicResource::None;

    // 사치 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    ELuxuryResource LuxuryResource = ELuxuryResource::None;

    // ========== 시설 정보 ==========

    // 타일 위 시설 RowName
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility")
    FName FacilityRowName = NAME_None;

    // 시설이 약탈됐는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility")
    bool bIsPillaged = false;

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

// 네트워크로 타일 한 칸을 보낼 때 씁니다.
USTRUCT()
struct CIVILIZATION_API FWorldTileNetData
{
	GENERATED_BODY()

	// 타일 hex
	UPROPERTY()
	FVector2D Hex = FVector2D::ZeroVector;

	// 지형·자원·시설
	UPROPERTY()
	FWorldSaveData Tile;
};

// ========== 세이브 슬롯 정보 ==========
USTRUCT(BlueprintType)
struct CIVILIZATION_API FSaveSlotInfo
{
    GENERATED_BODY()

    // 슬롯 번호 (1~5)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Slot")
    int32 SlotIndex = 0;

    // 유효한 세이브 파일이 있는지 여부
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

    // 호감도를 매기는 슬롯
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Attitude")
    int32 FromPlayerIndex = -1;

    // 호감도 대상 슬롯
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

    // 세이브 표시 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Info")
    FString SaveGameName = TEXT("");

    // 저장 시각
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Info")
    FDateTime SaveDateTime;

    // 저장한 슬롯 번호
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save Info")
    int32 SaveSlotIndex = 0;

    // ========== 게임 진행 상태 ==========

    // 현재 라운드. 세이브는 항상 슬롯 0 턴에서만 하므로 라운드만 저장합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    int32 CurrentRound = 1;

    // 메인메뉴 로드 때 월드를 다시 만들기 위한 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    FWorldConfig WorldConfig;

    // ========== 플레이어 데이터 ==========

    // 슬롯별 세이브 데이터
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Data")
    TArray<FPlayerSaveData> PlayerDataArray;

    // ========== 월드 타일 데이터 ==========

    // hex → 타일 지형·자원·시설
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Data")
    TMap<FVector2D, FWorldSaveData> WorldDataMap;

    // ========== 외교 데이터 ==========

    // 쌍별 전쟁·평화·동맹
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy Data")
    TMap<FDiplomacyPairKey, FDiplomacyPairState> DiplomacyStateMap;

    // 외교 액션 기록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy Data")
    TArray<FDiplomacyAction> DiplomacyActionHistory;

    // From→To 호감도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy Data")
    TMap<FAttitudeKey, int32> Attitudes;

    // 다음에 쓸 외교 액션 ID
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Diplomacy Data")
    int32 NextActionId = 1;

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
    // 파일에 쓰는 최상위 세이브 데이터
    UPROPERTY(VisibleAnywhere, Category = "Save Data")
    FGameSaveData SaveData;

    USuperSaveGame()
    {
    }
};
