// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CityStruct.h"
#include "GameFramework/Actor.h"
#include "CityComponent.generated.h"

class UDataTable;

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

    UFUNCTION(BlueprintCallable, Category = "City Initialization")
    void InitializeFromDataTable(FName RowName);

    // 데이터 접근
    UFUNCTION(BlueprintCallable, Category = "City Data")
    FCityData GetCityData() const { return m_CityData; }

    UFUNCTION(BlueprintCallable, Category = "City Data")
    FCityCurrentStat GetCurrentStat() const { return m_CurrentStat; }

    UFUNCTION(BlueprintCallable, Category = "City Data")
    TArray<EBuildingType> GetBuiltBuildings() const { return m_CityData.BuiltBuildings; }

    // 건물 관리
    UFUNCTION(BlueprintCallable, Category = "Building Management")
    void AddBuilding(EBuildingType BuildingType);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    void RemoveBuilding(EBuildingType BuildingType);

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    bool HasBuilding(EBuildingType BuildingType) const;

    UFUNCTION(BlueprintCallable, Category = "Building Management")
    FBuildingData GetBuildingData(EBuildingType BuildingType) const;

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
    void StartBuildingProduction(EBuildingType BuildingType, int32 ProductionCost);

    UFUNCTION(BlueprintCallable, Category = "Building Production")
    void UpdateBuildingProductionProgress(int32 Production);

    UFUNCTION(BlueprintCallable, Category = "Building Production")
    bool CanCompleteBuildingProduction() const;

    UFUNCTION(BlueprintCallable, Category = "Building Production")
    EBuildingType CompleteBuildingProduction();

    // 생산 시스템 - 유닛
    UFUNCTION(BlueprintCallable, Category = "Unit Production")
    void StartUnitProduction(FName UnitName, int32 ProductionCost);

    UFUNCTION(BlueprintCallable, Category = "Unit Production")
    void UpdateUnitProductionProgress(int32 Production);

    UFUNCTION(BlueprintCallable, Category = "Unit Production")
    bool CanCompleteUnitProduction() const;

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

    // 내부 함수들
    void LoadBuildingDataTable();
    void RecalculateYields();
    FBuildingData GetBuildingDataFromTable(EBuildingType BuildingType) const;
};

