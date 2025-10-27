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

// 도시 데이터 구조체
USTRUCT(BlueprintType)
struct CIVILIZATION_API FCityData
{
    GENERATED_BODY()

    // 기본 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "City Info")
    FString CityName;                                                      // 도시 이름

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

    // 게임 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    int32 OwnerPlayerID = -1;                                              // 소유자 플레이어 ID

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    bool bIsOccupied = false;                                               // 점령되었는지 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Game State")
    int32 OccupierPlayerID = -1;                                           // 점령자 플레이어 ID

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

    FCityData()
    {
        CityName = TEXT("New City");
        FoodYield = 0;
        ProductionYield = 0;
        GoldYield = 0;
        ScienceYield = 0;
        FaithYield = 0;
        BuiltBuildings.Empty();
        OwnerPlayerID = -1;
        bIsOccupied = false;
        OccupierPlayerID = -1;
        CurrentlyProducing = EBuildingType::None;
        ProductionProgress = 0;
        ProductionCost = 0;
        bIsProducingUnit = false;
        ProducingUnitName = NAME_None;
        UnitProductionProgress = 0;
        UnitProductionCost = 0;
    }
};

