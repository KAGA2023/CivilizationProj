// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WorldStruct.h"
#include "SuperPlayerState.generated.h"

class UWorldComponent;

// 자원 게이지 시스템
USTRUCT(BlueprintType)
struct CIVILIZATION_API FResourceGauge
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gauge")
    int32 CurrentAmount = 0; // 현재 게이지 양

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gauge")
    int32 MaxAmount = 100; // 최대 게이지 양

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resource Gauge")
    bool bIsFull = false; // 게이지가 가득 찼는지 여부

    FResourceGauge()
    {
        CurrentAmount = 0;
        MaxAmount = 100;
        bIsFull = false;
    }

    // 게이지에 양 추가 (가득 차면 자동으로 0이 되면서 true 반환)
    bool AddAmount(int32 Amount)
    {
        CurrentAmount += Amount;
        if (CurrentAmount >= MaxAmount)
        {
            CurrentAmount = 0; // 게이지가 가득 차면 0으로 리셋
            bIsFull = true;
            return true; // 인구 증가 가능
        }
        bIsFull = false;
        return false; // 아직 인구 증가 불가
    }

    // 게이지 비우기
    void Empty()
    {
        CurrentAmount = 0;
        bIsFull = false;
    }

    // 게이지 진행률 (0.0 ~ 1.0)
    float GetProgress() const
    {
        return MaxAmount > 0 ? (float)CurrentAmount / (float)MaxAmount : 0.0f;
    }
};

// 인구 증가 요구사항 데이터테이블
USTRUCT(BlueprintType)
struct CIVILIZATION_API FPopulationGrowthData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Growth")
    int32 PopulationLevel = 1; // 현재 인구수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Population Growth")
    int32 RequiredFood = 100; // 다음 인구 증가에 필요한 식량

    FPopulationGrowthData()
    {
        PopulationLevel = 1;
        RequiredFood = 100;
    }
};

UCLASS()
class CIVILIZATION_API ASuperPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ASuperPlayerState();

    // ========== 자원 시스템 ==========
    // 5종류 핵심 자원
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Resources")
    FResourceGauge FoodGauge; // 식량 게이지 (인구 증가용)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Production = 0; // 생산량 (유닛 건설용)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Gold = 0; // 골드 (범용 구매용)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Science = 0; // 과학 (기술 연구용)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Faith = 0; // 신앙 (전투력 추가)

    // ========== 타일 관리 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile Management")
    TArray<FVector2D> OwnedTileCoordinates; // 소유한 타일들의 좌표

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile Management")
    int32 TotalOwnedTiles = 7; // 소유한 총 타일 수

    // ========== 도시 관리 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "City Management")
    FVector2D CityCoordinate; // 도시 좌표 (개척자 없이 고정된 도시)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "City Management")
    bool bHasCity = false; // 도시 보유 여부

    // ========== 게임 진행 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Progress")
    int32 Population = 1; // 총 인구 수

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Progress")
    int32 LimitPopulation = 4; // 제한 인구 수

    // ========== 사치 자원 관리 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Luxury Resources")
    TMap<ELuxuryResource, int32> OwnedLuxuryResources; // 보유한 사치 자원 종류와 개수

    // ========== 데이터테이블 참조 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    TSoftObjectPtr<UDataTable> PopulationGrowthDataTable; // 인구 증가 데이터테이블

    // ========== 통계 정보 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    int32 TotalFoodProduced = 0; // 누적 식량 생산량

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    int32 TotalProductionProduced = 0; // 누적 생산량

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    int32 TotalGoldEarned = 0; // 누적 골드 획득량

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    int32 TotalScienceGained = 0; // 누적 과학 획득량

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statistics")
    int32 TotalFaithGained = 0; // 누적 신앙 획득량

protected:
    virtual void BeginPlay() override;

public:
    // ========== 자원 관리 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddFood(int32 Amount); // 식량 게이지에 추가

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddProduction(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddGold(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddScience(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddFaith(int32 Amount);


    // ========== 게이지 관리 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Gauge Management")
    int32 GetFoodGaugeCurrent() const { return FoodGauge.CurrentAmount; }

    UFUNCTION(BlueprintCallable, Category = "Gauge Management")
    int32 GetFoodGaugeMax() const { return FoodGauge.MaxAmount; }

    UFUNCTION(BlueprintCallable, Category = "Gauge Management")
    bool IsFoodGaugeFull() const { return FoodGauge.bIsFull; }

    UFUNCTION(BlueprintCallable, Category = "Gauge Management")
    float GetFoodGaugeProgress() const { return FoodGauge.GetProgress(); }

    UFUNCTION(BlueprintCallable, Category = "Gauge Management")
    void SetFoodGaugeMax(int32 MaxAmount); // 식량 게이지 최대값 설정 (인구 증가 시)

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    bool SpendProduction(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    bool SpendGold(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    bool SpendScience(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    bool SpendFaith(int32 Amount);

    // ========== 타일 관리 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void AddOwnedTile(FVector2D TileCoordinate, UWorldComponent* WorldComponent); // 좌표로 타일 추가 (소유 상태 업데이트)

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void RemoveOwnedTile(FVector2D TileCoordinate, UWorldComponent* WorldComponent); // 좌표로 타일 제거 (소유 상태 업데이트)

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    void ClearAllOwnedTiles(UWorldComponent* WorldComponent); // 모든 타일 제거 (소유 상태 업데이트)

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    TArray<FVector2D> GetOwnedTileCoordinates() const { return OwnedTileCoordinates; }

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    TArray<UWorldTile*> GetOwnedTiles(UWorldComponent* WorldComponent) const; // WorldComponent로 실제 타일들 가져오기

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    int32 GetOwnedTileCount() const { return OwnedTileCoordinates.Num(); }

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    bool IsTileOwned(FVector2D TileCoordinate) const; // 특정 좌표의 타일 소유 여부 확인

    // ========== 매턴 처리 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Turn Management")
    void ProcessTurnResources(); // 매턴 자원 생산 처리

    UFUNCTION(BlueprintCallable, Category = "Turn Management")
    void ProcessTurnPopulation(); // 매턴 인구 증가 처리 (데이터테이블 기반)

    // ========== 자원 생산량 계산 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Resource Calculation")
    int32 CalculateTotalFoodYield(UWorldComponent* WorldComponent) const; // 총 식량 생산량 계산

    UFUNCTION(BlueprintCallable, Category = "Resource Calculation")
    int32 CalculateTotalProductionYield(UWorldComponent* WorldComponent) const; // 총 생산량 계산

    UFUNCTION(BlueprintCallable, Category = "Resource Calculation")
    int32 CalculateTotalGoldYield(UWorldComponent* WorldComponent) const; // 총 골드 생산량 계산

    UFUNCTION(BlueprintCallable, Category = "Resource Calculation")
    int32 CalculateTotalScienceYield(UWorldComponent* WorldComponent) const; // 총 과학 생산량 계산

    UFUNCTION(BlueprintCallable, Category = "Resource Calculation")
    int32 CalculateTotalFaithYield(UWorldComponent* WorldComponent) const; // 총 신앙 생산량 계산

    // ========== 도시 관리 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "City Management")
    void SetCityCoordinate(FVector2D Coordinate); // 도시 좌표 설정

    UFUNCTION(BlueprintCallable, Category = "City Management")
    FVector2D GetCityCoordinate() const { return CityCoordinate; } // 도시 좌표 반환

    UFUNCTION(BlueprintCallable, Category = "City Management")
    bool HasCity() const { return bHasCity; } // 도시 보유 여부 확인

    UFUNCTION(BlueprintCallable, Category = "City Management")
    void RemoveCity(); // 도시 제거

    // ========== 게임 상태 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void InitializePlayer(); // 플레이어 초기화 (게임 시작/리셋 모두 사용)

    UFUNCTION(BlueprintCallable, Category = "Game State")
    int32 GetPopulation() const { return Population; }

    UFUNCTION(BlueprintCallable, Category = "Game State")
    int32 GetLimitPopulation() const { return LimitPopulation; }

    // ========== 사치 자원 관리 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Luxury Resources")
    void AddLuxuryResource(ELuxuryResource LuxuryResource, int32 Amount = 1); // 사치 자원 추가

    UFUNCTION(BlueprintCallable, Category = "Luxury Resources")
    void RemoveLuxuryResource(ELuxuryResource LuxuryResource, int32 Amount = 1); // 사치 자원 제거

    UFUNCTION(BlueprintCallable, Category = "Luxury Resources")
    void UpdatePopulationLimitFromLuxury(); // 사치 자원에 따른 인구 한계 업데이트

    UFUNCTION(BlueprintCallable, Category = "Luxury Resources")
    TMap<ELuxuryResource, int32> GetOwnedLuxuryResources() const { return OwnedLuxuryResources; }

    UFUNCTION(BlueprintCallable, Category = "Luxury Resources")
    int32 GetLuxuryResourceAmount(ELuxuryResource LuxuryResource) const; // 특정 사치 자원 개수 반환

    UFUNCTION(BlueprintCallable, Category = "Luxury Resources")
    bool HasLuxuryResource(ELuxuryResource LuxuryResource) const; // 특정 사치 자원 보유 여부 확인

};
