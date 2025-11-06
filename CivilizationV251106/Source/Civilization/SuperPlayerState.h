// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "WorldStruct.h"
#include "City/CityStruct.h"
#include "SuperPlayerState.generated.h"

class UWorldComponent;
class UCityComponent;

UCLASS()
class CIVILIZATION_API ASuperPlayerState : public APlayerState
{
    GENERATED_BODY()

public:
    ASuperPlayerState();

    // ========== 자원 시스템 ==========
    // 5종류 핵심 자원
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Food = 0; // 식량 (유닛 건설용)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Production = 0; // 생산량 (유닛 건설용)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Gold = 0; // 골드 (범용 구매용)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Science = 0; // 과학 (기술 연구용)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Resources")
    int32 Faith = 0; // 신앙 (전투력 추가)

    // ========== 플레이어 인덱스 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Info")
    int32 PlayerIndex = -1; // 플레이어 인덱스 (0=Player, 1~3=AI)

    // ========== 타일 관리 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile Management")
    TArray<FVector2D> OwnedTileCoordinates; // 소유한 타일들의 좌표

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tile Management")
    int32 TotalOwnedTiles = 7; // 소유한 총 타일 수

    // ========== 도시 관리 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "City Management")
    TObjectPtr<UCityComponent> CityComponent; // 도시 컴포넌트 참조

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "City Management")
    FVector2D CityCoordinate; // 도시 좌표 (개척자 없이 고정된 도시)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "City Management")
    bool bHasCity = false; // 도시 보유 여부

    // ========== 유닛 관리 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Unit Management")
    TArray<class AUnitCharacterBase*> OwnedUnits; // 이 플레이어가 소유한 유닛들

    // ========== 게임 진행 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Progress")
    int32 Population = 1; // 총 유닛 수

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Progress")
    int32 LimitPopulation = 4; // 제한 유닛 수

    // ========== 사치 자원 관리 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Luxury Resources")
    TMap<ELuxuryResource, int32> OwnedLuxuryResources; // 보유한 사치 자원 종류와 개수

protected:
    virtual void BeginPlay() override;

public:
    // ========== 자원 관리 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddFood(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddProduction(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddGold(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddScience(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    void AddFaith(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Resource Management")
    bool SpendFood(int32 Amount);

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
    void SetCityComponent(UCityComponent* InCityComponent); // 도시 컴포넌트 설정

    UFUNCTION(BlueprintCallable, Category = "City Management")
    UCityComponent* GetCityComponent() const { return CityComponent; } // 도시 컴포넌트 반환

    UFUNCTION(BlueprintCallable, Category = "City Management")
    void SetCityCoordinate(FVector2D Coordinate); // 도시 좌표 설정

    UFUNCTION(BlueprintCallable, Category = "City Management")
    FVector2D GetCityCoordinate() const { return CityCoordinate; } // 도시 좌표 반환

    UFUNCTION(BlueprintCallable, Category = "City Management")
    bool HasCity() const { return bHasCity; } // 도시 보유 여부 확인

    UFUNCTION(BlueprintCallable, Category = "City Management")
    void RemoveCity(); // 도시 제거

    // ========== 유닛 관리 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void AddOwnedUnit(class AUnitCharacterBase* Unit); // 소유 유닛 추가

    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void RemoveOwnedUnit(class AUnitCharacterBase* Unit); // 소유 유닛 제거

    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    TArray<class AUnitCharacterBase*> GetOwnedUnits() const { return OwnedUnits; } // 소유 유닛 목록 반환

    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    int32 GetOwnedUnitCount() const { return OwnedUnits.Num(); } // 소유 유닛 수 반환

    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void ClearAllOwnedUnits(); // 모든 소유 유닛 제거

    // ========== 도시 건물 생산 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "City Building Production")
    bool StartBuildingProduction(EBuildingType BuildingType); // 건물 생산 시작

    UFUNCTION(BlueprintCallable, Category = "City Building Production")
    bool ChangeBuildingProduction(EBuildingType NewBuildingType); // 건물 생산 변경 (진행도 초기화)

    // ========== 도시 유닛 생산 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "City Unit Production")
    bool StartUnitProduction(FName UnitName); // 유닛 생산 시작

    UFUNCTION(BlueprintCallable, Category = "City Unit Production")
    bool ChangeUnitProduction(FName NewUnitName); // 유닛 생산 변경 (진행도 초기화)

    // ========== 도시 건물 구매 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "City Building Purchase")
    bool PurchaseBuildingWithGold(EBuildingType BuildingType); // 골드로 건물 구매

    UFUNCTION(BlueprintCallable, Category = "City Building Purchase")
    bool PurchaseBuildingWithFaith(EBuildingType BuildingType); // 신앙으로 건물 구매

    UFUNCTION(BlueprintCallable, Category = "City Building Purchase")
    int32 GetBuildingGoldCost(EBuildingType BuildingType) const; // 건물 골드 구매 비용 조회

    UFUNCTION(BlueprintCallable, Category = "City Building Purchase")
    int32 GetBuildingFaithCost(EBuildingType BuildingType) const; // 건물 신앙 구매 비용 조회

    UFUNCTION(BlueprintCallable, Category = "City Building Purchase")
    bool CanPurchaseBuildingWithGold(EBuildingType BuildingType) const; // 골드 구매 가능 여부 확인

    UFUNCTION(BlueprintCallable, Category = "City Building Purchase")
    bool CanPurchaseBuildingWithFaith(EBuildingType BuildingType) const; // 신앙 구매 가능 여부 확인

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
