// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CityStruct.h"
#include "GameFramework/Actor.h"
#include "CityComponent.generated.h"

class UDataTable;
struct FUnitBaseStat;

// 건설 가능 건물·유닛 목록이 바뀌면 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnAvailableProductionsUpdated, TArray<FName>, AvailableBuildings, TArray<FName>, AvailableUnits);

// 생산을 시작하면 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionStarted, FName, ProductionID);

// 생산이 끝나면 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionCompleted, FName, ProductionID);

// 생산 진행도가 바뀌면 브로드캐스트합니다.
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

    // ========== 초기화 / 데이터 ==========

    // 도시 데이터로 현재 상태와 산출을 맞춥니다.
    UFUNCTION(BlueprintCallable, Category = "City Initialization")
    void InitFromCityData(const FCityData& InCityData);

    // 도시 기본 데이터를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Data")
    FCityData GetCityData() const { return m_CityData; }

    // 체력·생산 진행도를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Data")
    FCityCurrentStat GetCurrentStat() const { return m_CurrentStat; }

    // 이미 지은 건물 RowName 목록을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Data")
    TArray<FName> GetBuiltBuildings() const { return m_CityData.BuiltBuildings; }

    // ========== 건물 ==========

    // 건물을 목록에 넣고 산출을 다시 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Building Management")
    void AddBuilding(FName BuildingRowName);

    // 건물을 목록에서 빼고 산출을 다시 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Building Management")
    void RemoveBuilding(FName BuildingRowName);

    // 해당 건물을 이미 지었는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Building Management")
    bool HasBuilding(FName BuildingRowName) const;

    // 데이터 테이블에서 건물 한 줄을 읽습니다.
    UFUNCTION(BlueprintCallable, Category = "Building Management")
    FBuildingData GetBuildingDataFromTable(FName RowName) const;

    // 데이터 테이블에서 유닛 기본 스탯을 읽습니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    FUnitBaseStat GetUnitDataFromTable(FName RowName) const;

    // ========== 최종 산출 ==========

    // 건물을 포함한 최종 식량을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalFoodYield() const;

    // 건물을 포함한 최종 생산력을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalProductionYield() const;

    // 건물을 포함한 최종 골드를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalGoldYield() const;

    // 건물을 포함한 최종 과학을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalScienceYield() const;

    // 건물을 포함한 최종 신앙을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalFaithYield() const;

    // 건물을 포함한 최종 최대 체력을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Yields")
    int32 GetFinalMaxHealth() const;

    // ========== 건물 생산 ==========

    // 건물 생산을 시작하거나 바꾸고 진행도를 초기화합니다.
    UFUNCTION(BlueprintCallable, Category = "Building Production")
    void StartBuildingProduction(FName BuildingRowName);

    // 건물 생산 진행도에 생산력을 더합니다.
    UFUNCTION(BlueprintCallable, Category = "Building Production")
    void UpdateBuildingProductionProgress(int32 ProductionAmount);

    // 건물 생산 진행도를 직접 넣습니다. 세이브/로드용입니다.
    UFUNCTION(BlueprintCallable, Category = "Building Production")
    void SetProductionProgress(int32 Progress);

    // 건물 생산을 끝내고 완공된 건물 RowName을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Building Production")
    FName CompleteBuildingProduction();

    // ========== 유닛 생산 ==========

    // 유닛 생산을 시작하거나 바꾸고 진행도를 초기화합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Production")
    void StartUnitProduction(FName UnitName);

    // 유닛 생산 진행도에 식량을 더합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Production")
    FName UpdateUnitProductionProgress(int32 FoodAmount);

    // 식량 진행도를 직접 넣습니다. 세이브/로드용입니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Production")
    void SetFoodProgress(int32 Progress);

    // 유닛 생산을 끝내고 완공된 유닛 RowName을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Production")
    FName CompleteUnitProduction();

    // 지금 생산을 멈추고 진행도를 지웁니다.
    UFUNCTION(BlueprintCallable, Category = "Production")
    void StopCurrentProduction();

    // ========== 체력 ==========

    // 도시 체력을 깎습니다. 0이면 파괴합니다.
    UFUNCTION(BlueprintCallable, Category = "City Health")
    void TakeDamage(int32 DamageAmount);

    // 도시 체력을 회복합니다.
    UFUNCTION(BlueprintCallable, Category = "City Health")
    void Heal(int32 HealAmount);

    // 남은 체력을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Health")
    int32 GetCurrentHealth() const { return m_CurrentStat.RemainingHealth; }

    // 최대 체력을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Health")
    int32 GetMaxHealth() const;

    // 체력이 0 이하인지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "City Health")
    bool IsDead() const { return m_CurrentStat.RemainingHealth <= 0; }

    // ========== 생산 가능 목록 ==========

    // 지금 지을 수 있는 건물 목록을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Available Production")
    TArray<FName> GetAvailableBuildings() const { return AvailableBuildings; }

    // 지금 뽑을 수 있는 유닛 목록을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Available Production")
    TArray<FName> GetAvailableUnits() const { return AvailableUnits; }

    // 연구·자원에 맞춰 생산 가능 목록을 다시 만듭니다.
    UFUNCTION(BlueprintCallable, Category = "Available Production")
    void UpdateAvailableProductions();

    // 생산 가능 목록이 바뀌면 브로드캐스트합니다.
    UPROPERTY(BlueprintAssignable, Category = "Available Production")
    FOnAvailableProductionsUpdated OnAvailableProductionsUpdated;

    // 생산을 시작하면 브로드캐스트합니다.
    UPROPERTY(BlueprintAssignable, Category = "Production")
    FOnProductionStarted OnProductionStarted;

    // 생산이 끝나면 브로드캐스트합니다.
    UPROPERTY(BlueprintAssignable, Category = "Production")
    FOnProductionCompleted OnProductionCompleted;

    // 생산 진행도가 바뀌면 브로드캐스트합니다.
    UPROPERTY(BlueprintAssignable, Category = "Production")
    FOnProductionProgressChanged OnProductionProgressChanged;

protected:
    // ========== 내부 데이터 ==========

    // 도시 기본 데이터
    UPROPERTY(BlueprintReadOnly, Category = "City Data")
    FCityData m_CityData;

    // 체력과 생산 진행도
    UPROPERTY(BlueprintReadOnly, Category = "City Data")
    FCityCurrentStat m_CurrentStat;

    // 건물을 포함한 최종 산출
    UPROPERTY(BlueprintReadOnly, Category = "City Data")
    FCityFinalStat m_FinalStat;

    // 건물 데이터 테이블
    UPROPERTY()
    UDataTable* BuildingDataTable = nullptr;

    // 유닛 스테이터스 데이터 테이블
    UPROPERTY()
    UDataTable* UnitStatusTable = nullptr;

    // 지금 지을 수 있는 건물
    UPROPERTY(BlueprintReadOnly, Category = "Available Production")
    TArray<FName> AvailableBuildings;

    // 지금 뽑을 수 있는 유닛
    UPROPERTY(BlueprintReadOnly, Category = "Available Production")
    TArray<FName> AvailableUnits;

    // 건물 데이터 테이블을 로드합니다.
    void LoadBuildingDataTable();

    // 유닛 스테이터스 테이블을 로드합니다.
    void LoadUnitStatusTable();

    // 건물 보너스를 반영해 최종 산출을 다시 계산합니다.
    void RecalculateYields();

private:
    // ========== 도시 파괴 ==========

    // 체력이 0이 되면 도시를 제거합니다.
    void OnCityDestroyed();
};
