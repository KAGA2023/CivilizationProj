// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CityStruct.generated.h"

// 건물 타입 열거형
UENUM(BlueprintType)
enum class EBuildingType : uint8
{
    None                    UMETA(DisplayName = "None"),
    // 과학 건물
    Library                 UMETA(DisplayName = "Library"),                  // 도서관
    University              UMETA(DisplayName = "University"),             // 대학교
    ResearchLab             UMETA(DisplayName = "Research Lab"),            // 연구소
    
    // 문화 건물
    Monument                UMETA(DisplayName = "Monument"),                // 기념비
    Museum                  UMETA(DisplayName = "Museum"),                  // 박물관
    BroadcastCenter         UMETA(DisplayName = "Broadcast Center"),        // 방송센터
    
    // 종교 건물
    Shrine                  UMETA(DisplayName = "Shrine"),                  // 성소
    Temple                  UMETA(DisplayName = "Temple"),                  // 사원
    
    // 산업 건물
    Workshop                UMETA(DisplayName = "Workshop"),                // 작업장
    Factory                 UMETA(DisplayName = "Factory"),                 // 공장
    
    // 상업 건물
    Market                  UMETA(DisplayName = "Market"),                   // 시장
    Bank                    UMETA(DisplayName = "Bank"),                    // 은행
    
    // 군사 건물
    Barracks                UMETA(DisplayName = "Barracks"),                // 병영
    Armory                  UMETA(DisplayName = "Armory"),                  // 무기고
};

// 건물 데이터 구조체 (데이터 테이블용)
USTRUCT(BlueprintType)
struct CIVILIZATION_API FBuildingData : public FTableRowBase
{
    GENERATED_BODY()

    // 기본 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Info")
    EBuildingType BuildingType = EBuildingType::None;                     // 건물 타입

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Info")
    int32 MaxHealth = 0;                                                  // 건물 최대 체력

    // 생산량 증가 (도시에 건설 시 추가되는 생산량)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yields")
    int32 FoodYield = 0;                                                    // 식량 생산량 증가

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yields")
    int32 ProductionYield = 0;                                             // 생산력 증가

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yields")
    int32 GoldYield = 0;                                                    // 골드 생산량 증가

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yields")
    int32 ScienceYield = 0;                                                 // 과학 생산량 증가

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Yields")
    int32 FaithYield = 0;                                                   // 신앙 생산량 증가

    // 건설 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
    int32 ProductionCost = 0;                                               // 건설에 필요한 생산력

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cost")
    int32 FoodCost = 0;                                                     // 건설에 필요한 식량

    // 즉시 구매 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Purchase Cost")
    int32 GoldCost = 0;                                                      // 골드 구매 비용

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Purchase Cost")
    int32 FaithCost = 0;                                                     // 신앙심 구매 비용

    // 필요 기술
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Info")
    FName RequiredTechnology = NAME_None;                                   // 필요 기술

    // 건물 이름 및 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Info")
    FString BuildingName;                                                   // 건물 이름

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Info")
    FString Description;                                                    // 건물 설명

    FBuildingData()
    {
        BuildingType = EBuildingType::None;
        BuildingName = TEXT("New Building");
        Description = TEXT("");
        RequiredTechnology = NAME_None;
        MaxHealth = 0;
        FoodYield = 0;
        ProductionYield = 0;
        GoldYield = 0;
        ScienceYield = 0;
        FaithYield = 0;
        ProductionCost = 0;
        FoodCost = 0;
        GoldCost = 0;
        FaithCost = 0;
    }
};

// 도시 데이터 구조체
USTRUCT(BlueprintType)
struct CIVILIZATION_API FCityData
{
    GENERATED_BODY()

    // 기본 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Info")
    FString CityName;                                                      // 도시 이름

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Info")
    int32 MaxHealth = 0;                                                   // 도시 최대 체력

    // 생산량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Yields")
    int32 FoodYield = 0;                                                   // 식량 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Yields")
    int32 ProductionYield = 0;                                             // 생산력

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Yields")
    int32 GoldYield = 0;                                                    // 골드 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Yields")
    int32 ScienceYield = 0;                                                 // 과학 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Yields")
    int32 FaithYield = 0;                                                   // 신앙 생산량

    // 건물 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Buildings")
    TArray<EBuildingType> BuiltBuildings;                                  // 건설된 건물들

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

// 도시 현재 상태 구조체 (게임 실행 중 변화하는 값들)
USTRUCT(BlueprintType)
struct CIVILIZATION_API FCityCurrentStat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Status")
    int32 RemainingHealth = 0;                                             // 남은 체력

    // 건물 생산 중
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    EBuildingType CurrentlyProducing = EBuildingType::None;                // 현재 생산 중인 건물

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 ProductionProgress = 0;                                          // 현재 생산 진행도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 ProductionCost = 0;                                              // 목표 생산 비용

    // 유닛 생산 중
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    bool bIsProducingUnit = false;                                         // 유닛 생산 중인지

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    FName ProducingUnitName = NAME_None;                                   // 생산 중인 유닛 이름

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 UnitProductionProgress = 0;                                      // 유닛 생산 진행도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production")
    int32 UnitProductionCost = 0;                                          // 유닛 생산 비용

    FCityCurrentStat()
    {
        RemainingHealth = 0;
        CurrentlyProducing = EBuildingType::None;
        ProductionProgress = 0;
        ProductionCost = 0;
        bIsProducingUnit = false;
        ProducingUnitName = NAME_None;
        UnitProductionProgress = 0;
        UnitProductionCost = 0;
    }
};

// 도시 최종 스테이터스 구조체 (계산된 값들 - 건물 포함)
USTRUCT(BlueprintType)
struct CIVILIZATION_API FCityFinalStat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 FoodYield = 0;                   // 최종 식량 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 ProductionYield = 0;             // 최종 생산력

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 GoldYield = 0;                   // 최종 골드 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 ScienceYield = 0;               // 최종 과학 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 FaithYield = 0;                  // 최종 신앙 생산량

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Final Stats")
    int32 MaxHealth = 0;                   // 최종 최대 체력

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

