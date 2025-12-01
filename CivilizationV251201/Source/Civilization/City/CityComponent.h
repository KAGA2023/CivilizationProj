// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CityStruct.h"
#include "GameFramework/Actor.h"
#include "CityComponent.generated.h"

class UDataTable;
struct FUnitBaseStat;

// 건설 가능 목록 업데이트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAvailableProductionsUpdated, TArray<FName>, AvailableBuildings, TArray<FName>, AvailableUnits);

// 생산 시작 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionStarted, FName, ProductionID);

// 생산 완료 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionCompleted, FName, ProductionID);

// 생산 진행도 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProductionProgressChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CIVILIZATION_API UCityComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCityComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 초기화 함수들
    UFUNCTION(BlueprintCallable, Category = "City Initialization")
    void InitFromCityData(const FCityData& InCityData);

    // 데이터 접근
    UFUNCTION(BlueprintCallable, Category = "City Data")
    FCityData GetCityData() const { return m_CityData; }

    UFUNCTION(BlueprintCallable, Category = "City Data")
    FCityCurrentStat GetCurrentStat() const { return m_CurrentStat; }

    UFUNCTION(BlueprintCallable, Category = "City Data")
    TArray<FName> GetBuiltBuildings() const { return m_CityData.BuiltBuildings; }

    // 건물 관리
    UFUNCTION(BlueprintCallable, Category = "Building Management")
    void AddBuilding(FName BuildingRowName);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    void RemoveBuilding(FName BuildingRowName);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    bool HasBuilding(FName BuildingRowName) const;

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    FBuildingData GetBuildingDataFromTable(FName RowName) const;

    // 유닛 데이터 조회
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    FUnitBaseStat GetUnitDataFromTable(FName RowName) const;

    // 최종 생산량 계산 (건물 포함)
    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalFoodYield() const;

    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalProductionYield() const;

    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalGoldYield() const;

    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalScienceYield() const;

    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalFaithYield() const;

    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalMaxHealth() const;

    // 생산 시스템 - 건물
    UFUNCTION(BlueprintCallable, Category = "Building Production")
    void StartBuildingProduction(FName BuildingRowName); // 건물 생산 시작/변경 (진행도 초기화)

    UFUNCTION(BlueprintCallable, Category = "Building Production")
    void UpdateBuildingProductionProgress(int32 ProductionAmount);

    UFUNCTION(BlueprintCallable, Category = "Building Production")
    FName CompleteBuildingProduction();

    // 생산 시스템 - 유닛
    UFUNCTION(BlueprintCallable, Category = "Unit Production")
    void StartUnitProduction(FName UnitName); // 유닛 생산 시작/변경 (진행도 초기화)

    UFUNCTION(BlueprintCallable, Category = "Unit Production")
    FName UpdateUnitProductionProgress(int32 FoodAmount);

    UFUNCTION(BlueprintCallable, Category = "Unit Production")
    FName CompleteUnitProduction();

    // 체력 관리
    UFUNCTION(BlueprintCallable, Category = "City Health")
    void TakeDamage(int32 DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "City Health")
    void Heal(int32 HealAmount);

    UFUNCTION(BlueprintCallable, Category = "City Health")
    int32 GetCurrentHealth() const { return m_CurrentStat.RemainingHealth; }

    UFUNCTION(BlueprintCallable, Category = "City Health")
    int32 GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, Category = "City Health")
    bool IsDead() const { return m_CurrentStat.RemainingHealth <= 0; }

    // 건설 가능 목록 조회
    UFUNCTION(BlueprintCallable, Category = "Available Production")
    TArray<FName> GetAvailableBuildings() const { return AvailableBuildings; }

    UFUNCTION(BlueprintCallable, Category = "Available Production")
    TArray<FName> GetAvailableUnits() const { return AvailableUnits; }

    // 건설 가능 목록 업데이트
    UFUNCTION(BlueprintCallable, Category = "Available Production")
    void UpdateAvailableProductions();

    // 건설 가능 목록 업데이트 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Available Production")
    FOnAvailableProductionsUpdated OnAvailableProductionsUpdated;

    // 생산 시작 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Production")
    FOnProductionStarted OnProductionStarted;

    // 생산 완료 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Production")
    FOnProductionCompleted OnProductionCompleted;

    // 생산 진행도 변경 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Production")
    FOnProductionProgressChanged OnProductionProgressChanged;

protected:
    // 기본 도시 데이터
    UPROPERTY(BlueprintReadOnly, Category = "City Data")
    FCityData m_CityData;

    // 현재 상태
    UPROPERTY(BlueprintReadOnly, Category = "City Data")
    FCityCurrentStat m_CurrentStat;

    // 계산된 최종 스테이터스 (건물 포함)
    UPROPERTY(BlueprintReadOnly, Category = "City Data")
    FCityFinalStat m_FinalStat;

    // 건물 데이터 테이블
    UPROPERTY()
    UDataTable* BuildingDataTable = nullptr;

    // 유닛 스테이터스 데이터 테이블
    UPROPERTY()
    UDataTable* UnitStatusTable = nullptr;

    // 건설 가능 목록
    UPROPERTY(BlueprintReadOnly, Category = "Available Production")
    TArray<FName> AvailableBuildings;

    UPROPERTY(BlueprintReadOnly, Category = "Available Production")
    TArray<FName> AvailableUnits;

    // 내부 함수들
    void LoadBuildingDataTable();
    void LoadUnitStatusTable();
    void RecalculateYields();
};

