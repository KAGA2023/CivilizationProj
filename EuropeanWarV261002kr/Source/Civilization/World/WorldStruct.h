// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "WorldStruct.generated.h"

// ========== 지형 / 기후 / 땅 ==========

// 땅 또는 바다
UENUM(BlueprintType)
enum class ETerrainType : uint8
{
    None        UMETA(DisplayName = "None"),
    Land        UMETA(DisplayName = "Land"),
    Ocean       UMETA(DisplayName = "Ocean")        // 이동 불가 + 보너스 없음
};

// 땅에만 적용하는 기후
UENUM(BlueprintType)
enum class EClimateType : uint8
{
    None        UMETA(DisplayName = "None"),
    Temperate   UMETA(DisplayName = "Temperate"),    // 온대
    Desert      UMETA(DisplayName = "Desert"),       // 사막
    Tundra      UMETA(DisplayName = "Tundra")        // 툰드라
};

// 땅에만 적용하는 지형
UENUM(BlueprintType)
enum class ELandType : uint8
{
    None        UMETA(DisplayName = "None"),
    Plains      UMETA(DisplayName = "Plains"),       // 평지
    Hills       UMETA(DisplayName = "Hills"),        // 언덕
    Mountains   UMETA(DisplayName = "Mountains")     // 산
};


// ========== 자원 ==========

// 보너스 / 전략 / 사치
UENUM(BlueprintType)
enum class EResourceCategory : uint8
{
    None            UMETA(DisplayName = "None"),
    Bonus           UMETA(DisplayName = "Bonus"),     // 보너스 자원
    Strategic       UMETA(DisplayName = "Strategic"), // 전략 자원
    Luxury          UMETA(DisplayName = "Luxury")     // 사치 자원
};

// 기본 산출을 올리는 보너스 자원
UENUM(BlueprintType)
enum class EBonusResource : uint8
{
    None            UMETA(DisplayName = "None"),
    Wheat           UMETA(DisplayName = "Wheat"),
    Corn            UMETA(DisplayName = "Corn"),
    Chicken         UMETA(DisplayName = "Chicken"),
    Horse           UMETA(DisplayName = "Horse"),
    Deer            UMETA(DisplayName = "Deer"),
    Copper          UMETA(DisplayName = "Copper")
};

// 특수 유닛·건물에 쓰는 전략 자원
UENUM(BlueprintType)
enum class EStrategicResource : uint8
{
    None            UMETA(DisplayName = "None"),
    Iron            UMETA(DisplayName = "Iron")
};

// 행복도를 올리는 사치 자원
UENUM(BlueprintType)
enum class ELuxuryResource : uint8
{
    None            UMETA(DisplayName = "None"),
    Tiger           UMETA(DisplayName = "Tiger"),
    Diamond         UMETA(DisplayName = "Diamond"),
    Golden          UMETA(DisplayName = "Golden"),
    Silver          UMETA(DisplayName = "Silver"),
    Jade            UMETA(DisplayName = "Jade"),
    Watermelon      UMETA(DisplayName = "Watermelon"),
    Pumpkin         UMETA(DisplayName = "Pumpkin"),
    Sunflower       UMETA(DisplayName = "Sunflower"),
    Tomato          UMETA(DisplayName = "Tomato")
};

// ========== 기후 데이터 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FClimateData : public FTableRowBase
{
    GENERATED_BODY()

    // 이 줄의 기후 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    EClimateType ClimateType = EClimateType::Temperate;

    // 기본 식량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseFoodYield = 0;

    // 기본 생산력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseProductionYield = 0;

    // 기본 골드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseGoldYield = 0;

    // 기본 과학
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseScienceYield = 0;

    // 기본 신앙
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseFaithYield = 0;

    // 이동 비용 증가량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 MovementCost = 0;

    // 기본 전투 보너스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseCombatBonus = 0;

    // 기후 타일 바닥 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    UStaticMesh* TileMesh = nullptr;

    // 기후 숲 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    UStaticMesh* ForestMesh = nullptr;

    FClimateData()
    {
        ClimateType = EClimateType::Temperate;
        BaseFoodYield = 0;
        BaseProductionYield = 0;
        BaseGoldYield = 0;
        BaseScienceYield = 0;
        BaseFaithYield = 0;
        MovementCost = 0;
        BaseCombatBonus = 0;
        TileMesh = nullptr;
        ForestMesh = nullptr;
    }
};

// ========== 땅 타입 데이터 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FLandTypeData : public FTableRowBase
{
    GENERATED_BODY()

    // 이 줄의 땅 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    ELandType LandType = ELandType::Plains;

    // 식량 보너스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 FoodBonus = 0;

    // 생산력 보너스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 ProductionBonus = 0;

    // 골드 보너스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 GoldBonus = 0;

    // 과학 보너스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 ScienceBonus = 0;

    // 신앙 보너스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 FaithBonus = 0;

    // 이동 비용 증가량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 MovementCost = 0;

    // 전투 보너스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 CombatBonus = 0;

    FLandTypeData()
    {
        LandType = ELandType::Plains;
        FoodBonus = 0;
        ProductionBonus = 0;
        GoldBonus = 0;
        ScienceBonus = 0;
        FaithBonus = 0;
        MovementCost = 0;
        CombatBonus = 0;
    }
};

// ========== 보너스 자원 데이터 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FBonusResourceData : public FTableRowBase
{
    GENERATED_BODY()

    // 이 줄의 보너스 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    EBonusResource BonusResource = EBonusResource::None;

    // 자원 아이콘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    TSoftObjectPtr<UTexture2D> ResourceIcon;

    // 식량 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    int32 FoodYield = 0;

    // 생산력 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    int32 ProductionYield = 0;

    // 골드 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    int32 GoldYield = 0;

    // 과학 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    int32 ScienceYield = 0;

    // 신앙 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    int32 FaithYield = 0;

    // 나올 수 있는 기후
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    TArray<EClimateType> CompatibleClimates;

    // 나올 수 있는 지형
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    TArray<ELandType> CompatibleLandTypes;

    // 숲이 있어야 하는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    bool bRequiresForest = false;

    // 생성 확률
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    float SpawnProbability = 0.1f;

    // 자원 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    UStaticMesh* ResourceMesh = nullptr;

    // 이 자원 타일에 시설을 지었을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    UStaticMesh* FacilityMesh = nullptr;

    // 이 자원 타일 시설이 약탈됐을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource")
    UStaticMesh* PillagedMesh = nullptr;

    // 숲+자원 합본 메시 (시설 없음)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource | Forest+Resource")
    UStaticMesh* ForestResourceMesh = nullptr;

    // 숲+자원 타일에 시설을 지었을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource | Forest+Resource")
    UStaticMesh* ForestFacilityMesh = nullptr;

    // 숲+자원 타일 시설이 약탈됐을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Bonus Resource | Forest+Resource")
    UStaticMesh* ForestFacilityPillagedMesh = nullptr;

    FBonusResourceData()
    {
        BonusResource = EBonusResource::None;
        FoodYield = 0;
        ProductionYield = 0;
        GoldYield = 0;
        ScienceYield = 0;
        FaithYield = 0;
        CompatibleClimates.Empty();
        CompatibleLandTypes.Empty();
        bRequiresForest = false;
        SpawnProbability = 0.1f;
        ResourceMesh = nullptr;
        FacilityMesh = nullptr;
        PillagedMesh = nullptr;
        ForestResourceMesh = nullptr;
        ForestFacilityMesh = nullptr;
        ForestFacilityPillagedMesh = nullptr;
        ResourceIcon = nullptr;
    }
};

// ========== 전략 자원 데이터 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FStrategicResourceData : public FTableRowBase
{
    GENERATED_BODY()

    // 이 줄의 전략 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    EStrategicResource StrategicResource = EStrategicResource::None;

    // 자원 아이콘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    TSoftObjectPtr<UTexture2D> ResourceIcon;

    // 식량 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    int32 FoodYield = 0;

    // 생산력 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    int32 ProductionYield = 0;

    // 골드 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    int32 GoldYield = 0;

    // 과학 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    int32 ScienceYield = 0;

    // 신앙 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    int32 FaithYield = 0;

    // 나올 수 있는 기후
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    TArray<EClimateType> CompatibleClimates;

    // 나올 수 있는 지형
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    TArray<ELandType> CompatibleLandTypes;

    // 숲이 있어야 하는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    bool bRequiresForest = false;

    // 생성 확률
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    float SpawnProbability = 0.1f;

    // 자원 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    UStaticMesh* ResourceMesh = nullptr;

    // 이 자원 타일에 시설을 지었을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    UStaticMesh* FacilityMesh = nullptr;

    // 이 자원 타일 시설이 약탈됐을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource")
    UStaticMesh* PillagedMesh = nullptr;

    // 숲+자원 합본 메시 (시설 없음)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource | Forest+Resource")
    UStaticMesh* ForestResourceMesh = nullptr;

    // 숲+자원 타일에 시설을 지었을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource | Forest+Resource")
    UStaticMesh* ForestFacilityMesh = nullptr;

    // 숲+자원 타일 시설이 약탈됐을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Strategic Resource | Forest+Resource")
    UStaticMesh* ForestFacilityPillagedMesh = nullptr;

    FStrategicResourceData()
    {
        StrategicResource = EStrategicResource::None;
        FoodYield = 0;
        ProductionYield = 0;
        GoldYield = 0;
        ScienceYield = 0;
        FaithYield = 0;
        CompatibleClimates.Empty();
        CompatibleLandTypes.Empty();
        bRequiresForest = false;
        SpawnProbability = 0.1f;
        ResourceMesh = nullptr;
        FacilityMesh = nullptr;
        PillagedMesh = nullptr;
        ForestResourceMesh = nullptr;
        ForestFacilityMesh = nullptr;
        ForestFacilityPillagedMesh = nullptr;
        ResourceIcon = nullptr;
    }
};

// ========== 사치 자원 데이터 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FLuxuryResourceData : public FTableRowBase
{
    GENERATED_BODY()

    // 이 줄의 사치 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    ELuxuryResource LuxuryResource = ELuxuryResource::None;

    // 자원 아이콘
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    TSoftObjectPtr<UTexture2D> ResourceIcon;

    // 식량 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    int32 FoodYield = 0;

    // 생산력 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    int32 ProductionYield = 0;

    // 골드 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    int32 GoldYield = 0;

    // 과학 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    int32 ScienceYield = 0;

    // 신앙 산출
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    int32 FaithYield = 0;

    // 나올 수 있는 기후
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    TArray<EClimateType> CompatibleClimates;

    // 나올 수 있는 지형
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    TArray<ELandType> CompatibleLandTypes;

    // 숲이 있어야 하는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    bool bRequiresForest = false;
    
    // 생성 확률
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    float SpawnProbability = 0.1f;

    // 자원 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    UStaticMesh* ResourceMesh = nullptr;

    // 이 자원 타일에 시설을 지었을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    UStaticMesh* FacilityMesh = nullptr;

    // 이 자원 타일 시설이 약탈됐을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource")
    UStaticMesh* PillagedMesh = nullptr;

    // 숲+자원 합본 메시 (시설 없음)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource | Forest+Resource")
    UStaticMesh* ForestResourceMesh = nullptr;

    // 숲+자원 타일에 시설을 지었을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource | Forest+Resource")
    UStaticMesh* ForestFacilityMesh = nullptr;

    // 숲+자원 타일 시설이 약탈됐을 때 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Luxury Resource | Forest+Resource")
    UStaticMesh* ForestFacilityPillagedMesh = nullptr;

    FLuxuryResourceData()
    {
        LuxuryResource = ELuxuryResource::None;
        FoodYield = 0;
        ProductionYield = 0;
        GoldYield = 0;
        ScienceYield = 0;
        FaithYield = 0;
        CompatibleClimates.Empty();
        CompatibleLandTypes.Empty();
        bRequiresForest = false;
        SpawnProbability = 0.1f;
        ResourceMesh = nullptr;
        FacilityMesh = nullptr;
        PillagedMesh = nullptr;
        ForestResourceMesh = nullptr;
        ForestFacilityMesh = nullptr;
        ForestFacilityPillagedMesh = nullptr;
        ResourceIcon = nullptr;
    }
};

// ========== 타일 모디파이어 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FTileModifier
{
    GENERATED_BODY()

    // 식량 증가량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 AddFood = 0;

    // 생산력 증가량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 AddProduction = 0;

    // 골드 증가량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 AddGold = 0;

    // 과학 증가량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 AddScience = 0;

    // 신앙 증가량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 AddFaith = 0;

    // 이동 비용 증가량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 MovementCost = 0;

    // 전투 보너스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 CombatBonus = 0;

    bool operator==(const FTileModifier& Other) const
    {
        return AddFood == Other.AddFood && 
               AddProduction == Other.AddProduction && 
               AddGold == Other.AddGold && 
               AddScience == Other.AddScience && 
               AddFaith == Other.AddFaith &&
               MovementCost == Other.MovementCost &&
               CombatBonus == Other.CombatBonus;
    }

    FTileModifier()
    {
        AddFood = 0;
        AddProduction = 0;
        AddGold = 0;
        AddScience = 0;
        AddFaith = 0;
        MovementCost = 0;
        CombatBonus = 0;
    }
};

// ========== 타일 데이터 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FTileData
{
    GENERATED_BODY()

    // ========== 위치 ==========

    // hex 좌표
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
    FVector2D GridPosition = FVector2D::ZeroVector;

    // 월드 좌표
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
    FVector WorldPosition = FVector::ZeroVector;

    // ========== 지형 ==========

    // 땅 또는 바다
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Type")
    ETerrainType TerrainType = ETerrainType::Land;

    // 기후대
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Type")
    EClimateType ClimateType = EClimateType::Temperate;

    // 땅 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Type")
    ELandType LandType = ELandType::Plains;

    // ========== 자원 ==========

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

    // ========== 지형 특성 ==========

    // 숲이 있는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Terrain Feature")
    bool bHasForest = false;

    // ========== 게임 상태 ==========

    // 누가 소유했는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    bool bIsOwned = false;

    // 소유 문명 슬롯. 0=호스트/싱글, 1=1v1 참가자, 나머지는 AI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    int32 OwnerPlayerID = -1;

    // ========== 캐시된 산출 ==========

    // 캐시된 식량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedFoodYield = 0;

    // 캐시된 생산력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedProductionYield = 0;

    // 캐시된 골드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedGoldYield = 0;

    // 캐시된 과학
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedScienceYield = 0;

    // 캐시된 신앙
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedFaithYield = 0;

    // 캐시된 이동 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedMovementCost = 1;

    // 캐시된 전투 보너스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedCombatBonus = 0;

    FTileData()
    {
        GridPosition = FVector2D::ZeroVector;
        WorldPosition = FVector::ZeroVector;
        TerrainType = ETerrainType::Land;
        ClimateType = EClimateType::Temperate;
        LandType = ELandType::Plains;
        ResourceCategory = EResourceCategory::None;
        BonusResource = EBonusResource::None;
        StrategicResource = EStrategicResource::None;
        LuxuryResource = ELuxuryResource::None;
        bHasForest = false;
        bIsOwned = false;
        OwnerPlayerID = -1;
        CachedFoodYield = 0;
        CachedProductionYield = 0;
        CachedGoldYield = 0;
        CachedScienceYield = 0;
        CachedFaithYield = 0;
        CachedMovementCost = 1;
        CachedCombatBonus = 0;
    }
};

// ========== 월드 설정 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FWorldConfig
{
    GENERATED_BODY()

    // 월드 반지름 (타일 수)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 WorldRadius = 25;

    // 총 문명 슬롯 수. 0=호스트/싱글, 1=1v1 참가자, 나머지는 AI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 PlayerCount = 4;

    // 바다 비율
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float OceanPercentage = 0.2f;

    // ========== 기후 비율 ==========

    // 온대 비율 (땅에만 적용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float TemperatePercentage = 0.6f;

    // 사막 비율 (땅에만 적용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float DesertPercentage = 0.2f;

    // 툰드라 비율 (땅에만 적용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float TundraPercentage = 0.2f;

    // ========== 지형 비율 ==========

    // 평지 비율 (땅에만 적용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float PlainsPercentage = 0.85f;

    // 언덕 비율 (땅에만 적용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float HillsPercentage = 0.1f;

    // 산 비율 (땅에만 적용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float MountainPercentage = 0.05f;

    // 숲 비율 (땅에만 적용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float ForestPercentage = 0.3f;

    FWorldConfig()
    {
        WorldRadius = 25;
        PlayerCount = 4;
        
        // 바다 비율
        OceanPercentage = 0.2f;
        
        // 기후대 비율 (땅에만 적용)
        TemperatePercentage = 0.6f;
        DesertPercentage = 0.2f;
        TundraPercentage = 0.2f;
        
        // 지형 타입 비율 (땅에만 적용)
        PlainsPercentage = 0.85f;
        HillsPercentage = 0.1f;
        MountainPercentage = 0.05f;
        
        // 지형 특성 비율
        ForestPercentage = 0.3f;
    }
};

///////////////////////////Object//////////////////////////////

// 한 칸의 지형·자원·소유·산출을 담는 타일 오브젝트
UCLASS(Blueprintable)
class CIVILIZATION_API UWorldTile : public UObject
{
    GENERATED_BODY()

protected:
    // 이 칸의 지형·자원·소유 데이터
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Tile")
    FTileData m_TileData;

    // 시설 등으로 붙는 산출 보정
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Tile")
    TArray<FTileModifier> m_TileModifiers;

    // 지금 선택된 타일인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Tile")
    bool bIsSelected = false;

public:
    UWorldTile();

    // ========== 타일 데이터 ==========

    // 타일 데이터 전체를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FTileData GetTileData() const { return m_TileData; }

    // 타일 데이터 전체를 넣습니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetTileData(const FTileData& NewTileData) { m_TileData = NewTileData; }

    // hex 좌표를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FVector2D GetGridPosition() const { return m_TileData.GridPosition; }

    // hex 좌표를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetGridPosition(FVector2D NewPosition) { m_TileData.GridPosition = NewPosition; }

    // 월드 좌표를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FVector GetWorldPosition() const { return m_TileData.WorldPosition; }

    // 월드 좌표를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetWorldPosition(FVector NewPosition) { m_TileData.WorldPosition = NewPosition; }

    // ========== 지형 ==========

    // 땅/바다를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    ETerrainType GetTerrainType() const { return m_TileData.TerrainType; }

    // 땅/바다를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetTerrainType(ETerrainType NewType) { m_TileData.TerrainType = NewType; }

    // 기후를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    EClimateType GetClimateType() const { return m_TileData.ClimateType; }

    // 기후를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetClimateType(EClimateType NewType) { m_TileData.ClimateType = NewType; }

    // 땅 타입을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    ELandType GetLandType() const { return m_TileData.LandType; }

    // 땅 타입을 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetLandType(ELandType NewType) { m_TileData.LandType = NewType; }


    // ========== 자원 ==========

    // 자원 카테고리를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    EResourceCategory GetResourceCategory() const { return m_TileData.ResourceCategory; }

    // 자원 카테고리를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetResourceCategory(EResourceCategory NewCategory) { m_TileData.ResourceCategory = NewCategory; }

    // 보너스 자원을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    EBonusResource GetBonusResource() const { return m_TileData.BonusResource; }

    // 보너스 자원을 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetBonusResource(EBonusResource NewResource) { m_TileData.BonusResource = NewResource; }

    // 전략 자원을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    EStrategicResource GetStrategicResource() const { return m_TileData.StrategicResource; }

    // 전략 자원을 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetStrategicResource(EStrategicResource NewResource) { m_TileData.StrategicResource = NewResource; }

    // 사치 자원을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    ELuxuryResource GetLuxuryResource() const { return m_TileData.LuxuryResource; }

    // 사치 자원을 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetLuxuryResource(ELuxuryResource NewResource) { m_TileData.LuxuryResource = NewResource; }

    // ========== 모디파이어 ==========

    // 산출 보정을 넣습니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    void AddTileModifier(const FTileModifier& Modifier);

    // 산출 보정을 뺍니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    void RemoveTileModifier(const FTileModifier& Modifier);

    // 모든 산출 보정을 지웁니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    void ClearAllModifiers();

    // 기본+보정 식량을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalFoodYield() const;

    // 기본+보정 생산력을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalProductionYield() const;

    // 기본+보정 골드를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalGoldYield() const;

    // 기본+보정 과학을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalScienceYield() const;

    // 기본+보정 신앙을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalFaithYield() const;

    // 기본+보정 이동 비용을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalMovementCost() const;

    // 기본+보정 전투 보너스를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalCombatBonus() const;


    // ========== 소유 / 숲 ==========

    // 소유된 타일인지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    bool IsOwned() const { return m_TileData.bIsOwned; }

    // 소유 여부를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetOwned(bool bOwned) { m_TileData.bIsOwned = bOwned; }

    // 소유 문명 슬롯을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    int32 GetOwnerPlayerID() const { return m_TileData.OwnerPlayerID; }

    // 소유 문명 슬롯을 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetOwnerPlayerID(int32 PlayerID) { m_TileData.OwnerPlayerID = PlayerID; }

    // 숲이 있는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    bool HasForest() const { return m_TileData.bHasForest; }

    // 숲 여부를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetHasForest(bool bHasForest) { m_TileData.bHasForest = bHasForest; }


    // ========== 캐시된 산출 ==========

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    int32 GetFoodYield() const { return m_TileData.CachedFoodYield; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    int32 GetProductionYield() const { return m_TileData.CachedProductionYield; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    int32 GetGoldYield() const { return m_TileData.CachedGoldYield; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    int32 GetScienceYield() const { return m_TileData.CachedScienceYield; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    int32 GetFaithYield() const { return m_TileData.CachedFaithYield; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    int32 GetMovementCost() const { return m_TileData.CachedMovementCost; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    int32 GetCombatBonus() const { return m_TileData.CachedCombatBonus; }

    // ========== 선택 ==========

    // 선택된 타일인지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    bool IsSelected() const { return bIsSelected; }

    // 선택 여부를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetSelected(bool bSelected) { bIsSelected = bSelected; }

    // ========== 유틸 ==========

    // 통과 가능한 타일인지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    bool IsPassable() const;

    // 자원이 있는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    bool HasResource() const;

    // 기후 이름 문자열을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetClimateTypeName() const;

    // 땅 타입 이름 문자열을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetLandTypeName() const;

    // 보너스 자원 이름 문자열을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetBonusResourceName() const;

    // 전략 자원 이름 문자열을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetStrategicResourceName() const;

    // 사치 자원 이름 문자열을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetLuxuryResourceName() const;

    // 지금 있는 자원 이름을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetResourceName() const;

    // "온대 평지"처럼 전체 타일 이름을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetFullTileName() const;
};
