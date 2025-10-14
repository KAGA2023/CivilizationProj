// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/DataTable.h"
#include "WorldStruct.generated.h"

// 지형 타입 (땅/바다)
UENUM(BlueprintType)
enum class ETerrainType : uint8
{
    None        UMETA(DisplayName = "None"),
    Land        UMETA(DisplayName = "Land"),
    Ocean       UMETA(DisplayName = "Ocean")        // 이동 불가 + 보너스 없음
};

// 기후 타입 (땅에만 적용)
UENUM(BlueprintType)
enum class EClimateType : uint8
{
    None        UMETA(DisplayName = "None"),
    Temperate   UMETA(DisplayName = "Temperate"),    // 온대
    Desert      UMETA(DisplayName = "Desert"),       // 사막
    Tundra      UMETA(DisplayName = "Tundra")        // 툰드라
};

// 땅 타입 (땅에만 적용)
UENUM(BlueprintType)
enum class ELandType : uint8
{
    None        UMETA(DisplayName = "None"),
    Plains      UMETA(DisplayName = "Plains"),       // 평지
    Hills       UMETA(DisplayName = "Hills"),        // 언덕
    Forest      UMETA(DisplayName = "Forest"),       // 숲
    Mountains   UMETA(DisplayName = "Mountains")     // 산
};


// 자원 카테고리 (보너스/전략/사치)
UENUM(BlueprintType)
enum class EResourceCategory : uint8
{
    None            UMETA(DisplayName = "None"),
    Bonus           UMETA(DisplayName = "Bonus"),     // 보너스 자원
    Strategic       UMETA(DisplayName = "Strategic"), // 전략 자원
    Luxury          UMETA(DisplayName = "Luxury")     // 사치 자원
};

// 보너스 자원 (기본 생산량 증가)
UENUM(BlueprintType)
enum class EBonusResource : uint8
{
    None            UMETA(DisplayName = "None"),
    Wheat           UMETA(DisplayName = "Wheat"),     // 밀
    Rice            UMETA(DisplayName = "Rice"),      // 쌀
    Maize           UMETA(DisplayName = "Maize"),     // 옥수수
    Cattle          UMETA(DisplayName = "Cattle"),    // 소
    Sheep           UMETA(DisplayName = "Sheep"),     // 양
    Deer            UMETA(DisplayName = "Deer"),      // 사슴
    Copper          UMETA(DisplayName = "Copper"),    // 구리
    Stone           UMETA(DisplayName = "Stone"),     // 돌
    Bananas         UMETA(DisplayName = "Bananas")    // 바나나
};

// 전략 자원 (특수 유닛/건물 건설에 필요)
UENUM(BlueprintType)
enum class EStrategicResource : uint8
{
    None            UMETA(DisplayName = "None"),
    Horses          UMETA(DisplayName = "Horses"),    // 말
    Iron            UMETA(DisplayName = "Iron")       // 철
};

// 사치 자원 (행복도 증가)
UENUM(BlueprintType)
enum class ELuxuryResource : uint8
{
    None            UMETA(DisplayName = "None"),
    Furs            UMETA(DisplayName = "Furs"),      // 모피(툰드라)
    Diamonds        UMETA(DisplayName = "Diamonds"),  // 다이아몬드(확률이 제일 적음음)
    Salt            UMETA(DisplayName = "Salt"),      // 소금(사막/툰드라)
    Silver          UMETA(DisplayName = "Silver"),    // 은(사막/툰드라)
    Tobacco         UMETA(DisplayName = "Tobacco"),   // 담배
    Silk            UMETA(DisplayName = "Silk"),      // 비단
    Wine            UMETA(DisplayName = "Wine"),      // 와인
    Tea             UMETA(DisplayName = "Tea"),       // 차
    Coffee          UMETA(DisplayName = "Coffee"),    // 커피
    Spices          UMETA(DisplayName = "Spices")     // 향신료(사막)
};

// 기후대 데이터테이블 구조체
USTRUCT(BlueprintType)
struct CIVILIZATION_API FClimateData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    EClimateType ClimateType = EClimateType::Temperate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseFoodYield = 0; // 기본 식량 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseProductionYield = 0; // 기본 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseGoldYield = 0; // 기본 골드 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseScienceYield = 0; // 기본 과학 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseFaithYield = 0; // 기본 신앙 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    float MovementCostMultiplier = 1.0f; // 이동 비용 배수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Climate")
    int32 BaseDefenseBonus = 0; // 기본 방어 보너스

    FClimateData()
    {
        ClimateType = EClimateType::Temperate;
        BaseFoodYield = 0;
        BaseProductionYield = 0;
        BaseGoldYield = 0;
        BaseScienceYield = 0;
        BaseFaithYield = 0;
        MovementCostMultiplier = 1.0f;
        BaseDefenseBonus = 0;
    }
};

// 땅 타입 데이터테이블 구조체
USTRUCT(BlueprintType)
struct CIVILIZATION_API FLandTypeData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    ELandType LandType = ELandType::Plains;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 FoodBonus = 0; // 식량 보너스

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 ProductionBonus = 0; // 생산량 보너스

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 GoldBonus = 0; // 골드 보너스

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 ScienceBonus = 0; // 과학 보너스

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 FaithBonus = 0; // 신앙 보너스

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    float MovementCostMultiplier = 1.0f; // 이동 비용 배수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Land Type")
    int32 DefenseBonus = 0; // 방어 보너스

    FLandTypeData()
    {
        LandType = ELandType::Plains;
        FoodBonus = 0;
        ProductionBonus = 0;
        GoldBonus = 0;
        ScienceBonus = 0;
        FaithBonus = 0;
        MovementCostMultiplier = 1.0f;
        DefenseBonus = 0;
    }
};

// 타일 모디파이어 (타일 생산량 변화용)
USTRUCT(BlueprintType)
struct CIVILIZATION_API FTileModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 AddFood = 0; // 식량 증가량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 AddProduction = 0; // 생산량 증가량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 AddGold = 0; // 골드 증가량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 AddScience = 0; // 과학 증가량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 AddFaith = 0; // 신앙 증가량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    float MovementCostMultiplier = 1.0f; // 이동 비용 배수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    int32 DefenseBonus = 0; // 방어 보너스

    // 모디파이어 비교용 연산자
    bool operator==(const FTileModifier& Other) const
    {
        return AddFood == Other.AddFood && 
               AddProduction == Other.AddProduction && 
               AddGold == Other.AddGold && 
               AddScience == Other.AddScience && 
               AddFaith == Other.AddFaith &&
               MovementCostMultiplier == Other.MovementCostMultiplier &&
               DefenseBonus == Other.DefenseBonus;
    }

    FTileModifier()
    {
        AddFood = 0;
        AddProduction = 0;
        AddGold = 0;
        AddScience = 0;
        AddFaith = 0;
        MovementCostMultiplier = 1.0f;
        DefenseBonus = 0;
    }
};

// 타일 데이터 구조체 (계층적 테이블 방식)
USTRUCT(BlueprintType)
struct CIVILIZATION_API FTileData
{
    GENERATED_BODY()

    // 위치 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
    FVector2D GridPosition = FVector2D::ZeroVector; // 그리드 좌표

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Position")
    FVector WorldPosition = FVector::ZeroVector; // 월드 좌표

    // 지형 타입 (데이터테이블 참조용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Type")
    ETerrainType TerrainType = ETerrainType::Land; // 지형 타입 (땅/바다)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Type")
    FName ClimateTypeID = NAME_None; // 기후대 ID ("Desert", "Temperate", "Tundra" 등)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Type")
    FName LandTypeID = NAME_None; // 땅 타입 ID ("Plains", "Hills", "Forest", "Mountains")

    // 자원 정보 (동적 생성)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    EResourceCategory ResourceCategory = EResourceCategory::None; // 자원 카테고리

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    EBonusResource BonusResource = EBonusResource::None; // 보너스 자원

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    EStrategicResource StrategicResource = EStrategicResource::None; // 전략 자원

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
    ELuxuryResource LuxuryResource = ELuxuryResource::None; // 사치 자원

    // 게임 상태 (런타임 데이터)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    bool bIsOwned = false; // 소유되었는지 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    int32 OwnerPlayerID = -1; // 소유자 플레이어 ID

    // 계산된 값들 (캐시용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedFoodYield = 0; // 캐시된 식량 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedProductionYield = 0; // 캐시된 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedGoldYield = 0; // 캐시된 골드 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedScienceYield = 0; // 캐시된 과학 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedFaithYield = 0; // 캐시된 신앙 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    float CachedMovementCost = 1.0f; // 캐시된 이동 비용

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Calculated")
    int32 CachedDefenseBonus = 0; // 캐시된 방어 보너스

    FTileData()
    {
        GridPosition = FVector2D::ZeroVector;
        WorldPosition = FVector::ZeroVector;
        TerrainType = ETerrainType::Land;
        ClimateTypeID = NAME_None;
        LandTypeID = NAME_None;
        ResourceCategory = EResourceCategory::None;
        BonusResource = EBonusResource::None;
        StrategicResource = EStrategicResource::None;
        LuxuryResource = ELuxuryResource::None;
        bIsOwned = false;
        OwnerPlayerID = -1;
        CachedFoodYield = 0;
        CachedProductionYield = 0;
        CachedGoldYield = 0;
        CachedScienceYield = 0;
        CachedFaithYield = 0;
        CachedMovementCost = 1.0f;
        CachedDefenseBonus = 0;
    }
};

// 월드 설정 구조체
USTRUCT(BlueprintType)
struct CIVILIZATION_API FWorldConfig
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    int32 WorldRadius = 25; // 월드 반지름 (타일 수)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float OceanPercentage = 0.3f; // 바다 비율

    // 기후대 비율 (땅에만 적용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float TemperatePercentage = 0.6f; // 온대 비율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float DesertPercentage = 0.2f; // 사막 비율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float TundraPercentage = 0.2f; // 툰드라 비율

    // 지형 타입 비율 (땅에만 적용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float PlainsPercentage = 0.5f; // 평지 비율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float HillsPercentage = 0.2f; // 언덕 비율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float ForestPercentage = 0.2f; // 숲 비율

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    float MountainPercentage = 0.1f; // 산 비율

    FWorldConfig()
    {
        WorldRadius = 25;
        
        // 바다 비율
        OceanPercentage = 0.3f;
        
        // 기후대 비율 (땅에만 적용)
        TemperatePercentage = 0.6f;
        DesertPercentage = 0.2f;
        TundraPercentage = 0.2f;
        
        // 지형 타입 비율 (땅에만 적용)
        PlainsPercentage = 0.5f;
        HillsPercentage = 0.25f;
        ForestPercentage = 0.15f;
        MountainPercentage = 0.1f;
    }
};

///////////////////////////Object//////////////////////////////

// 월드 타일 오브젝트 클래스
UCLASS(Blueprintable)
class CIVILIZATION_API UWorldTile : public UObject
{
    GENERATED_BODY()

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Tile")
    FTileData m_TileData; // 타일 데이터

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Tile")
    TArray<FTileModifier> m_TileModifiers; // 타일 모디파이어 배열

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World Tile")
    bool bIsSelected = false; // 현재 선택되었는지 여부


public:
    UWorldTile();

    // 타일 데이터 접근
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FTileData GetTileData() const { return m_TileData; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetTileData(const FTileData& NewTileData) { m_TileData = NewTileData; }

    // 그리드 위치
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FVector2D GetGridPosition() const { return m_TileData.GridPosition; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetGridPosition(FVector2D NewPosition) { m_TileData.GridPosition = NewPosition; }

    // 월드 위치
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FVector GetWorldPosition() const { return m_TileData.WorldPosition; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetWorldPosition(FVector NewPosition) { m_TileData.WorldPosition = NewPosition; }

    // 지형 타입
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    ETerrainType GetTerrainType() const { return m_TileData.TerrainType; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetTerrainType(ETerrainType NewType) { m_TileData.TerrainType = NewType; }

    // 기후 타입 (ID 기반)
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FName GetClimateTypeID() const { return m_TileData.ClimateTypeID; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetClimateTypeID(FName NewID) { m_TileData.ClimateTypeID = NewID; }

    // 땅 타입 (ID 기반)
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FName GetLandTypeID() const { return m_TileData.LandTypeID; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetLandTypeID(FName NewID) { m_TileData.LandTypeID = NewID; }


    // 자원 카테고리
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    EResourceCategory GetResourceCategory() const { return m_TileData.ResourceCategory; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetResourceCategory(EResourceCategory NewCategory) { m_TileData.ResourceCategory = NewCategory; }

    // 보너스 자원
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    EBonusResource GetBonusResource() const { return m_TileData.BonusResource; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetBonusResource(EBonusResource NewResource) { m_TileData.BonusResource = NewResource; }

    // 전략 자원
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    EStrategicResource GetStrategicResource() const { return m_TileData.StrategicResource; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetStrategicResource(EStrategicResource NewResource) { m_TileData.StrategicResource = NewResource; }

    // 사치 자원
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    ELuxuryResource GetLuxuryResource() const { return m_TileData.LuxuryResource; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetLuxuryResource(ELuxuryResource NewResource) { m_TileData.LuxuryResource = NewResource; }

    // 타일 모디파이어 시스템
    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    void AddTileModifier(const FTileModifier& Modifier); // 모디파이어 추가

    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    void RemoveTileModifier(const FTileModifier& Modifier); // 모디파이어 제거

    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    void ClearAllModifiers(); // 모든 모디파이어 제거

    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalFoodYield() const; // 총 식량 생산량 (기본 + 모디파이어)

    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalProductionYield() const; // 총 생산량 (기본 + 모디파이어)

    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalGoldYield() const; // 총 골드 생산량 (기본 + 모디파이어)

    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalScienceYield() const; // 총 과학 생산량 (기본 + 모디파이어)

    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalFaithYield() const; // 총 신앙 생산량 (기본 + 모디파이어)

    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    float GetTotalMovementCost() const; // 총 이동 비용 (기본 * 모디파이어 배수)

    UFUNCTION(BlueprintCallable, Category = "Tile Modifier")
    int32 GetTotalDefenseBonus() const; // 총 방어 보너스 (기본 + 모디파이어)


    // 소유 상태
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    bool IsOwned() const { return m_TileData.bIsOwned; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetOwned(bool bOwned) { m_TileData.bIsOwned = bOwned; }

    // 소유자 ID
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    int32 GetOwnerPlayerID() const { return m_TileData.OwnerPlayerID; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetOwnerPlayerID(int32 PlayerID) { m_TileData.OwnerPlayerID = PlayerID; }


    // 캐시된 생산량 접근 (데이터테이블 기반 계산된 값)
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
    float GetMovementCost() const { return m_TileData.CachedMovementCost; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    int32 GetDefenseBonus() const { return m_TileData.CachedDefenseBonus; }

    // 선택 상태
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    bool IsSelected() const { return bIsSelected; }

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    void SetSelected(bool bSelected) { bIsSelected = bSelected; }

    // 유틸리티 함수들
    UFUNCTION(BlueprintCallable, Category = "World Tile")
    bool IsPassable() const; // 통과 가능한지 여부

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    bool HasResource() const; // 자원이 있는지 여부

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetClimateTypeName() const; // 기후 타입 이름 반환

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetLandTypeName() const; // 땅 타입 이름 반환


    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetBonusResourceName() const; // 보너스 자원 이름 반환

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetStrategicResourceName() const; // 전략 자원 이름 반환

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetLuxuryResourceName() const; // 사치 자원 이름 반환

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetResourceName() const; // 현재 자원 이름 반환 (카테고리에 따라 적절한 자원 반환)

    UFUNCTION(BlueprintCallable, Category = "World Tile")
    FString GetFullTileName() const; // 전체 타일 이름 반환 (예: "온대 평지", "사막 언덕", "깊은 바다")
};
