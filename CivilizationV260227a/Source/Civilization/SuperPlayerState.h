// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "World/WorldStruct.h"
#include "City/CityStruct.h"
#include "SuperPlayerState.generated.h"

class UWorldComponent;
class UCityComponent;
class UResearchComponent;
class UTexture2D;

// 골드 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGoldChanged, int32, NewGold);

// 소유 타일 변경 델리게이트 (타일 구매 등)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOwnedTilesChanged);

// 인구 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPopulationChanged, int32, NewPopulation);

// 전략 자원 보유량 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnStrategicResourceStockChanged, EStrategicResource, Resource, int32, NewStock);

// 사치 자원 보유량 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnLuxuryResourceChanged, ELuxuryResource, Resource, int32, NewAmount);

// ========== 승리/패배 시스템 델리게이트 ==========
// 플레이어 승리 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerVictory);

// 플레이어 패배 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDefeated);

// AI 플레이어 패배 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAIPlayerDefeated, int32, DefeatedPlayerIndex);

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
    int32 PlayerIndex = -1; // 플레이어 인덱스 (0=Player, 1~N-1=AI)

    // ========== 국가 정보 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Info")
    FName CountryRowName = NAME_None; // 국가 데이터 테이블 RowName

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Info")
    FString CountryName = TEXT(""); // 국가 이름 (캐시)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Info")
    TSoftObjectPtr<UTexture2D> CountryLargeImg; // 국가 큰 이미지 (외교 UI 등에서 사용, 캐시)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Info")
    TSoftObjectPtr<UTexture2D> CountryKingImg; // 국가 왕 이미지 (캐시)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Player Info")
    FLinearColor BorderColor = FLinearColor::White; // 국경선 색상 (캐시)

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

    // ========== 기술 관리 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tech Management")
    TObjectPtr<UResearchComponent> ResearchComponent; // 기술 컴포넌트 참조

    // ========== 유닛 관리 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Unit Management")
    TArray<class AUnitCharacterBase*> OwnedUnits; // 이 플레이어가 소유한 유닛들

    // ========== 게임 진행 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Progress")
    int32 Population = 0; // 총 유닛 수

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Game Progress")
    int32 LimitPopulation = 4; // 제한 유닛 수

    // ========== 사치 자원 관리 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Luxury Resources")
    TMap<ELuxuryResource, int32> OwnedLuxuryResources; // 보유한 사치 자원 종류와 개수

    // ========== 전략 자원 관리 시스템 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Strategic Resources")
    TMap<EStrategicResource, int32> OwnedStrategicResources; // 보유한 전략 자원 종류와 개수 (시설 건설 여부)

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Strategic Resources")
    TMap<EStrategicResource, int32> OwnedStrategicResourceStocks; // 전략 자원 보유량 (최대 20개)

    // ========== 시설 건설 가능 목록 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Facility")
    TArray<FName> AvailableFacilities; // 건설 가능한 시설 목록 (RowName 배열)

    // 시설 데이터 테이블 참조 (임시로 직접 로드)
    UPROPERTY()
    class UDataTable* FacilityDataTable = nullptr;

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

    // ========== 타일 구매 시스템 ==========
    UFUNCTION(BlueprintCallable, Category = "Tile Purchase")
    bool CanPurchaseTile(FVector2D TileCoordinate, UWorldComponent* WorldComponent); // 타일 구매 가능 여부 확인

    UFUNCTION(BlueprintCallable, Category = "Tile Purchase")
    int32 CalculateTilePurchaseCost(FVector2D TileCoordinate, UWorldComponent* WorldComponent); // 타일 구매 비용 계산

    UFUNCTION(BlueprintCallable, Category = "Tile Purchase")
    bool PurchaseTile(FVector2D TileCoordinate, UWorldComponent* WorldComponent); // 타일 구매 실행

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

    // ========== 기술 관리 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Tech Management")
    void SetResearchComponent(UResearchComponent* InResearchComponent); // 기술 컴포넌트 설정

    UFUNCTION(BlueprintCallable, Category = "Tech Management")
    UResearchComponent* GetResearchComponent() const { return ResearchComponent; } // 기술 컴포넌트 반환

    // ========== 기술 연구 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Tech Research")
    bool StartTechResearch(FName TechRowName); // 기술 연구 시작/변경 (진행도 초기화)

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
    bool StartBuildingProduction(FName BuildingRowName); // 건물 생산 시작/변경 (진행도 초기화)

    // ========== 도시 유닛 생산 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "City Unit Production")
    bool StartUnitProduction(FName UnitName); // 유닛 생산 시작/변경 (진행도 초기화)

    // ========== 도시 건물 구매 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "City Building Purchase")
    bool PurchaseBuildingWithGold(FName BuildingRowName); // 골드로 건물 구매

    // ========== 도시 유닛 구매 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "City Unit Purchase")
    bool PurchaseUnitWithGold(FName UnitName); // 골드로 유닛 구매

    // ========== 국가 정보 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Player Info")
    FString GetCountryName() const { return CountryName; }

    UFUNCTION(BlueprintCallable, Category = "Player Info")
    void SetCountryName(const FString& NewCountryName) { CountryName = NewCountryName; }

    UFUNCTION(BlueprintCallable, Category = "Player Info")
    UTexture2D* GetCountryLargeImg() const { return CountryLargeImg.LoadSynchronous(); }

    UFUNCTION(BlueprintCallable, Category = "Player Info")
    void SetCountryLargeImg(TSoftObjectPtr<UTexture2D> NewCountryLargeImg) { CountryLargeImg = NewCountryLargeImg; }

    UFUNCTION(BlueprintCallable, Category = "Player Info")
    UTexture2D* GetCountryKingImg() const { return CountryKingImg.LoadSynchronous(); }

    UFUNCTION(BlueprintCallable, Category = "Player Info")
    void SetCountryKingImg(TSoftObjectPtr<UTexture2D> NewCountryKingImg) { CountryKingImg = NewCountryKingImg; }

    UFUNCTION(BlueprintCallable, Category = "Player Info")
    void LoadCountryDataFromTable(); // 국가 데이터 테이블에서 국가 정보 로드

    // ========== 게임 상태 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Game State")
    void InitializePlayer(); // 플레이어 초기화 (게임 시작/리셋 모두 사용)

    UFUNCTION(BlueprintCallable, Category = "Game State")
    int32 GetPopulation() const { return Population; }

    UFUNCTION(BlueprintCallable, Category = "Game State")
    int32 GetLimitPopulation() const { return LimitPopulation; }

    // ========== 델리게이트 ==========
    UPROPERTY(BlueprintAssignable, Category = "Delegates")
    FOnGoldChanged OnGoldChanged;

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
    FOnOwnedTilesChanged OnOwnedTilesChanged;

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
    FOnPopulationChanged OnPopulationChanged;

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
    FOnStrategicResourceStockChanged OnStrategicResourceStockChanged;

    UPROPERTY(BlueprintAssignable, Category = "Delegates")
    FOnLuxuryResourceChanged OnLuxuryResourceChanged;

    // ========== 승리/패배 시스템 델리게이트 ==========
    // 플레이어 승리 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Victory")
    FOnPlayerVictory OnPlayerVictoryDelegate;

    // 플레이어 패배 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Victory")
    FOnPlayerDefeated OnPlayerDefeatedDelegate;

    // AI 플레이어 패배 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Victory")
    FOnAIPlayerDefeated OnAIPlayerDefeatedDelegate;

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

    // ========== 전략 자원 관리 함수들 ==========
    UFUNCTION(BlueprintCallable, Category = "Strategic Resources")
    void AddStrategicResource(EStrategicResource StrategicResource, int32 Amount = 1); // 전략 자원 추가

    UFUNCTION(BlueprintCallable, Category = "Strategic Resources")
    void RemoveStrategicResource(EStrategicResource StrategicResource, int32 Amount = 1); // 전략 자원 제거

    UFUNCTION(BlueprintCallable, Category = "Strategic Resources")
    TMap<EStrategicResource, int32> GetOwnedStrategicResources() const { return OwnedStrategicResources; }

    UFUNCTION(BlueprintCallable, Category = "Strategic Resources")
    int32 GetStrategicResourceAmount(EStrategicResource StrategicResource) const; // 특정 전략 자원 개수 반환

    UFUNCTION(BlueprintCallable, Category = "Strategic Resources")
    bool HasStrategicResource(EStrategicResource StrategicResource) const; // 특정 전략 자원 보유 여부 확인

    UFUNCTION(BlueprintCallable, Category = "Strategic Resources")
    int32 GetStrategicResourceStock(EStrategicResource StrategicResource) const; // 전략 자원 보유량 반환

    UFUNCTION(BlueprintCallable, Category = "Strategic Resources")
    bool CanAffordStrategicResource(EStrategicResource StrategicResource, int32 Amount) const; // 전략 자원 보유량 충분 여부 확인

    UFUNCTION(BlueprintCallable, Category = "Strategic Resources")
    bool SpendStrategicResource(EStrategicResource StrategicResource, int32 Amount); // 전략 자원 소모

    // ========== 시설 건설 함수들 ==========
    // 건설 가능한 시설 목록 가져오기 (RowName 배열)
    UFUNCTION(BlueprintCallable, Category = "Facility")
    TArray<FName> GetAvailableFacilities() const { return AvailableFacilities; }

    // 건설 가능한 시설 목록 업데이트 (기술 시스템에서 호출)
    UFUNCTION(BlueprintCallable, Category = "Facility")
    void UpdateAvailableFacilities();

    // 플레이어가 건설 가능한 시설 목록 가져오기 (타일 조건 필터링)
    UFUNCTION(BlueprintCallable, Category = "Facility")
    TArray<struct FFacilityData> GetBuildableFacilities(class UWorldTile* TargetTile) const;

    // 시설 건설 요청
    UFUNCTION(BlueprintCallable, Category = "Facility")
    bool BuildFacility(FName FacilityRowName, FVector2D TileCoordinate);

    // ========== 승리/패배 시스템 ==========
    // 패배(아웃) 상태
    UPROPERTY(BlueprintReadOnly, Category = "Victory")
    bool bIsDefeated = false;
    
    // 패배 처리 (도시 파괴 시 호출)
    UFUNCTION(BlueprintCallable, Category = "Victory")
    void SetDefeated();
    
    // 생존 확인
    UFUNCTION(BlueprintCallable, Category = "Victory")
    bool IsAlive() const { return !bIsDefeated; }

private:
    // 플레이어(인간) 패배 처리
    void OnPlayerDefeated_Human();
    
    // AI 플레이어 패배 처리
    void OnPlayerDefeated_AI();
    
    // 패배한 플레이어 정리 (유닛/영토)
    void CleanupDefeatedPlayer();
};
