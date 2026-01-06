// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WorldStruct.h"
#include "WorldComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldGenerated, bool, bSuccess); // 월드 생성 완료 이벤트

// 타일 크기 상수 (고정값)
const float TILE_SIZE = 190.0f;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CIVILIZATION_API UWorldComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    // 월드 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    FWorldConfig WorldConfig; // 월드 생성 설정

    // 데이터테이블 참조
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* ClimateDataTable = nullptr; // 기후대 데이터테이블

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* LandTypeDataTable = nullptr; // 땅 타입 데이터테이블

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* BonusResourceDataTable = nullptr; // 보너스 자원 데이터테이블

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* StrategicResourceDataTable = nullptr; // 전략 자원 데이터테이블

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* LuxuryResourceDataTable = nullptr; // 사치 자원 데이터테이블


    // 타일 배열 (육각형 맵)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Data")
    TMap<FVector2D, UWorldTile*> HexTiles; // 육각형 좌표 → 타일 매핑


    // 월드 생성 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World State")
    bool bIsWorldGenerated = false; // 월드 생성 완료 여부

	// 시작 도시 좌표들 (도시 스폰은 WorldSpawner에서 처리)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Placement")
	TArray<FVector2D> StartingCityHexes;

public:
    UWorldComponent();

    // 이벤트 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "World Events")
    FOnWorldGenerated OnWorldGenerated;

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 월드 생성 및 관리
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld(); // 월드 생성 시작

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearWorld(); // 월드 초기화

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool IsWorldGenerated() const { return bIsWorldGenerated; } // 월드 생성 완료 여부

    // 타일 접근 및 관리
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    UWorldTile* GetTileAtHex(FVector2D HexPosition) const; // 육각형 좌표로 타일 가져오기

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    UWorldTile* CreateTileAtHex(FVector2D HexPosition); // 육각형 좌표에 타일 생성

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    bool RemoveTileAtHex(FVector2D HexPosition); // 육각형 좌표의 타일 제거

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    TArray<UWorldTile*> GetAllTiles() const; // 모든 타일 가져오기

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    TArray<UWorldTile*> GetTilesInRadius(FVector2D CenterHex, int32 Radius) const; // 반경 내 타일들 가져오기

    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    int32 GetTotalTileCount() const { return HexTiles.Num(); } // 총 타일 수

    // 육각형 좌표계 유틸리티
    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    TArray<FVector2D> GetHexNeighbors(FVector2D HexPosition) const; // 육각형 인접 타일들 가져오기

    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    int32 GetHexDistance(FVector2D Hex1, FVector2D Hex2) const; // 두 육각형 간 거리 계산

    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    TArray<FVector2D> GetHexesInRadius(FVector2D CenterHex, int32 Radius) const; // 반경 내 육각형 좌표들

    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    FVector HexToWorld(FVector2D HexPosition) const; // 육각형 좌표를 월드 좌표로 변환

    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    FVector2D WorldToHex(FVector WorldPosition) const; // 월드 좌표를 육각형 좌표로 변환

    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    bool IsValidHexPosition(FVector2D HexPosition) const; // 유효한 육각형 좌표인지 확인

    // 타일 호버 시스템
    UFUNCTION(BlueprintCallable, Category = "Tile Hover")
    void HandleTileHoverBegin(UWorldTile* HoveredTile); // 타일 호버 시작 처리

    UFUNCTION(BlueprintCallable, Category = "Tile Hover")
    void HandleTileHoverEnd(UWorldTile* HoveredTile); // 타일 호버 종료 처리

    // 월드 설정 관리
    UFUNCTION(BlueprintCallable, Category = "World Settings")
    FWorldConfig GetWorldConfig() const { return WorldConfig; } // 월드 설정 가져오기

    UFUNCTION(BlueprintCallable, Category = "World Settings")
    void SetWorldConfig(const FWorldConfig& NewSettings); // 월드 설정 변경

    UFUNCTION(BlueprintCallable, Category = "World Settings")
    int32 GetMapRadius() const { return WorldConfig.WorldRadius; } // 맵 반지름 가져오기

    UFUNCTION(BlueprintCallable, Category = "World Settings")
    float GetTileSize() const { return TILE_SIZE; } // 타일 크기 가져오기 (고정값)

    // 데이터테이블 관리
    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void SetClimateDataTable(UDataTable* DataTable) { ClimateDataTable = DataTable; } // 기후대 데이터테이블 설정

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void SetLandTypeDataTable(UDataTable* DataTable) { LandTypeDataTable = DataTable; } // 땅 타입 데이터테이블 설정

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void SetBonusResourceDataTable(UDataTable* DataTable) { BonusResourceDataTable = DataTable; } // 보너스 자원 데이터테이블 설정

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void SetStrategicResourceDataTable(UDataTable* DataTable) { StrategicResourceDataTable = DataTable; } // 전략 자원 데이터테이블 설정

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void SetLuxuryResourceDataTable(UDataTable* DataTable) { LuxuryResourceDataTable = DataTable; } // 사치 자원 데이터테이블 설정

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    UDataTable* GetClimateDataTable() const { return ClimateDataTable; } // 기후대 데이터테이블 가져오기

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    UDataTable* GetLandTypeDataTable() const { return LandTypeDataTable; } // 땅 타입 데이터테이블 가져오기

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    UDataTable* GetBonusResourceDataTable() const { return BonusResourceDataTable; } // 보너스 자원 데이터테이블 가져오기

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    UDataTable* GetStrategicResourceDataTable() const { return StrategicResourceDataTable; } // 전략 자원 데이터테이블 가져오기

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    UDataTable* GetLuxuryResourceDataTable() const { return LuxuryResourceDataTable; } // 사치 자원 데이터테이블 가져오기

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void LoadDataTables(); // 데이터테이블 로딩

	// ================= 도시 시작 위치 계산(데이터 전용) =================
	// 지형이 육지인 타일 중에서, 다른 도시와 최소 거리 3, 반경1 이웃 6칸 중 최소 5칸이 육지인 조건을 만족하는 좌표 선택
	// 선택된 좌표는 StartingCityHexes에 저장됨(스폰은 하지 않음)
	UFUNCTION(BlueprintCallable, Category = "City Placement")
	void GenerateCities(int32 NumCities = 4, int32 MinHexDistance = 5, int32 RequiredLandNeighbors = 5);

	UFUNCTION(BlueprintCallable, Category = "City Placement")
	TArray<FVector2D> GetStartingCityHexes() const { return StartingCityHexes; }

	// ================= 도시 좌표/조회 유틸 =================
	UFUNCTION(BlueprintCallable, Category = "City Placement")
	int32 GetCityCount() const { return StartingCityHexes.Num(); }

	UFUNCTION(BlueprintCallable, Category = "City Placement")
	bool IsCityAtHex(FVector2D Hex) const { return StartingCityHexes.Contains(Hex); }

	// ================= 도시 관리 훅 =================
	// 데이터 배열 수정 + 이벤트 브로드캐스트. 스폰/삭제는 외부(WorldSpawner)가 처리
	UFUNCTION(BlueprintCallable, Category = "City Placement")
	bool RemoveCityAt(FVector2D Hex);

    // 타일 생산량 계산
    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    void RecalculateTileYields(UWorldTile* Tile); // 특정 타일 생산량 재계산

    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    void RecalculateAllTileYields(); // 모든 타일 생산량 재계산

    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseFoodYield(UWorldTile* Tile) const; // 기본 식량 생산량 계산

    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseProductionYield(UWorldTile* Tile) const; // 기본 생산량 계산

    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseGoldYield(UWorldTile* Tile) const; // 기본 골드 생산량 계산

    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseScienceYield(UWorldTile* Tile) const; // 기본 과학 생산량 계산

    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseFaithYield(UWorldTile* Tile) const; // 기본 신앙 생산량 계산

    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseMovementCost(UWorldTile* Tile) const; // 기본 이동 비용 계산

    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateCombatBonus(UWorldTile* Tile) const; // 기본 전투 보너스 계산

    // 지형 생성 알고리즘
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrain(); // 지형 생성

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateResources(); // 자원 생성

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateLuxuryResources(TArray<UWorldTile*>& LandTiles); // 사치 자원 생성

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateStrategicResources(TArray<UWorldTile*>& LandTiles); // 전략 자원 생성

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBonusResources(TArray<UWorldTile*>& LandTiles); // 보너스 자원 생성

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateClimateZones(); // 기후대 생성

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateLandTypes(); // 땅 타입 생성

    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateForests(); // 숲 생성

    // 판게아 스타일 지형 생성 함수들
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GeneratePangaeaTerrain(); // 판게아 스타일 지형 생성

    // 통계 및 정보
    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetLandTileCount() const; // 땅 타일 수

    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetOceanTileCount() const; // 바다 타일 수

    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetResourceTileCount(EResourceCategory ResourceCategory) const; // 자원 타일 수

    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetClimateTileCount(EClimateType ClimateType) const; // 기후대 타일 수

    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetLandTypeTileCount(ELandType LandType) const; // 땅 타입별 타일 수

    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetForestTileCount() const; // 숲이 있는 타일 수

private:
    // 내부 헬퍼 함수들
    void InitializeHexTiles(); // 육각형 타일들 초기화
    UWorldTile* CreateNewTile(FVector2D HexPosition); // 새 타일 생성
    void DestroyTile(UWorldTile* Tile); // 타일 파괴
    bool IsResourceCompatibleWithTile(const TArray<EClimateType>& CompatibleClimates, const TArray<ELandType>& CompatibleLandTypes, bool bRequiresForest, UWorldTile* Tile) const; // 자원 호환성 확인
};
