// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "CityStruct.generated.h"

// 건물 카테고리
UENUM(BlueprintType)
enum class EBuildingType : uint8
{
    None                    UMETA(DisplayName = "None"),                    // 없음
    Agricultural            UMETA(DisplayName = "Agricultural"),            // 식량건물
    Industrial              UMETA(DisplayName = "Industrial"),              // 산업건물
    Commercial              UMETA(DisplayName = "Commercial"),              // 상업건물
    Scientific              UMETA(DisplayName = "Scientific"),              // 과학건물
    Military                UMETA(DisplayName = "Military"),                // 군사건물
    Mix                     UMETA(DisplayName = "Mix"),                     // 혼합건물
};

// 도시가 지금 생산 중인 대상
UENUM(BlueprintType)
enum class EProductionType : uint8
{
    None                    UMETA(DisplayName = "None"),
    Building                UMETA(DisplayName = "Building"),                // 건물 생산
    Unit                    UMETA(DisplayName = "Unit")                      // 유닛 생산
};

// 건물 데이터 테이블 한 줄
USTRUCT(BlueprintType)
struct CIVILIZATION_API FBuildingData : public FTableRowBase
{
    GENERATED_BODY()

    // ========== 기본 정보 ==========

    // 건물 카테고리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Info")
    EBuildingType BuildingType = EBuildingType::None;

    // 건물 표시 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Info")
    FString BuildingName;

    // 건물 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Info")
    FString Description;

    // 건물 아이콘 이미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Info")
    TSoftObjectPtr<UTexture2D> BuildingIcon;

    // ========== 전투 ==========

    // 건물이 도시에 더하는 최대 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    int32 MaxHealth = 0;

    // ========== 생산량 ==========

    // 도시에 더하는 식량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yields")
    int32 FoodYield = 0;

    // 도시에 더하는 생산력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yields")
    int32 ProductionYield = 0;

    // 도시에 더하는 골드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yields")
    int32 GoldYield = 0;

    // 도시에 더하는 과학
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yields")
    int32 ScienceYield = 0;

    // 도시에 더하는 신앙
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yields")
    int32 FaithYield = 0;

    // ========== 비용 ==========

    // 건설에 필요한 생산력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production & Cost")
    int32 ProductionCost = 0;

    // 골드로 즉시 살 때 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production & Cost")
    int32 GoldCost = 0;

    FBuildingData()
    {
        BuildingType = EBuildingType::None;
        BuildingName = TEXT("New Building");
        Description = TEXT("");
        MaxHealth = 0;
        FoodYield = 0;
        ProductionYield = 0;
        GoldYield = 0;
        ScienceYield = 0;
        FaithYield = 0;
        ProductionCost = 0;
        GoldCost = 0;
    }
};

// 도시의 기본 이름·산출·건물 목록
USTRUCT(BlueprintType)
struct CIVILIZATION_API FCityData
{
    GENERATED_BODY()

    // ========== 기본 정보 ==========

    // 도시 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Info")
    FString CityName;

    // 도시 최대 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Info")
    int32 MaxHealth = 0;

    // ========== 생산량 ==========

    // 식량 생산량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Yields")
    int32 FoodYield = 0;

    // 생산력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Yields")
    int32 ProductionYield = 0;

    // 골드 생산량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Yields")
    int32 GoldYield = 0;

    // 과학 생산량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Yields")
    int32 ScienceYield = 0;

    // 신앙 생산량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Yields")
    int32 FaithYield = 0;

    // ========== 건물 ==========

    // 이미 지은 건물 RowName. 같은 종류도 여러 개 가능합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buildings")
    TArray<FName> BuiltBuildings;

    FCityData()
    {
        CityName = TEXT("New City");
        MaxHealth = 0;
        FoodYield = 0;
        ProductionYield = 0;
        GoldYield = 0;
        ScienceYield = 0;
        FaithYield = 0;
        BuiltBuildings.Empty();
    }
};

// 게임 중 바뀌는 도시 체력과 생산 진행도
USTRUCT(BlueprintType)
struct CIVILIZATION_API FCityCurrentStat
{
    GENERATED_BODY()

    // ========== 체력 ==========

    // 남은 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Status")
    int32 RemainingHealth = 0;

    // ========== 생산 ==========

    // 지금 생산 중인 대상 (없음/건물/유닛)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EProductionType ProductionType = EProductionType::None;

    // 생산 중인 건물 또는 유닛 RowName
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FName ProductionName = NAME_None;

    // 건물 생산 진행도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 ProductionProgress = 0;

    // 건물 생산에 필요한 생산력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 ProductionCost = 0;

    // 유닛 생산 식량 진행도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 FoodProgress = 0;

    // 유닛 생산에 필요한 식량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 FoodCost = 0;

    FCityCurrentStat()
    {
        RemainingHealth = 0;
        ProductionType = EProductionType::None;
        ProductionName = NAME_None;
        ProductionProgress = 0;
        ProductionCost = 0;
        FoodProgress = 0;
        FoodCost = 0;
    }
};

// 건물을 포함한 최종 도시 산출
USTRUCT(BlueprintType)
struct CIVILIZATION_API FCityFinalStat
{
    GENERATED_BODY()

    // 최종 식량 생산량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 FoodYield = 0;

    // 최종 생산력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 ProductionYield = 0;

    // 최종 골드 생산량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 GoldYield = 0;

    // 최종 과학 생산량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 ScienceYield = 0;

    // 최종 신앙 생산량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 FaithYield = 0;

    // 최종 최대 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 MaxHealth = 0;

    FCityFinalStat()
    {
        FoodYield = 0;
        ProductionYield = 0;
        GoldYield = 0;
        ScienceYield = 0;
        FaithYield = 0;
        MaxHealth = 0;
    }
};
