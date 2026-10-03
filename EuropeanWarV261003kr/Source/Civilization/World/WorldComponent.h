// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WorldStruct.h"
#include "WorldComponent.generated.h"

// 월드 생성이 끝나면 성공 여부를 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWorldGenerated, bool, bSuccess);

// 타일 크기 상수 (고정값)
const float TILE_SIZE = 190.0f;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CIVILIZATION_API UWorldComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    // ========== 설정 / 데이터 테이블 ==========

    // 반지름·지형 비율·슬롯 수 등 월드 생성 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Settings")
    FWorldConfig WorldConfig;

    // 기후대 데이터 테이블
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* ClimateDataTable = nullptr;

    // 땅 타입 데이터 테이블
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* LandTypeDataTable = nullptr;

    // 보너스 자원 데이터 테이블
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* BonusResourceDataTable = nullptr;

    // 전략 자원 데이터 테이블
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* StrategicResourceDataTable = nullptr;

    // 사치 자원 데이터 테이블
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data Tables")
    UDataTable* LuxuryResourceDataTable = nullptr;


    // ========== 월드 데이터 ==========

    // hex → 타일 오브젝트
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World Data")
    TMap<FVector2D, UWorldTile*> HexTiles;


    // 월드 생성이 끝났는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "World State")
    bool bIsWorldGenerated = false;

	// 시작 도시 hex. 실제 스폰은 WorldSpawner가 합니다.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "City Placement")
	TArray<FVector2D> StartingCityHexes;

public:
    UWorldComponent();

    // 월드 생성이 끝나면 브로드캐스트합니다.
    UPROPERTY(BlueprintAssignable, Category = "World Events")
    FOnWorldGenerated OnWorldGenerated;

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========== 월드 생성 ==========

    // 새 월드를 생성합니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorld();

    // 세이브 타일로 월드를 다시 만듭니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateWorldFromSaveData(const TMap<FVector2D, struct FWorldSaveData>& WorldDataMap, const FWorldConfig& Config, const TArray<struct FPlayerSaveData>& PlayerDataArray);

    // 타일을 모두 지웁니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void ClearWorld();

    // 월드 생성이 끝났는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    bool IsWorldGenerated() const { return bIsWorldGenerated; }

    // ========== 타일 접근 ==========

    // 해당 hex의 타일을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    UWorldTile* GetTileAtHex(FVector2D HexPosition) const;

    // 해당 hex에 타일을 만듭니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    UWorldTile* CreateTileAtHex(FVector2D HexPosition);

    // 해당 hex의 타일을 제거합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    bool RemoveTileAtHex(FVector2D HexPosition);

    // 모든 타일을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    TArray<UWorldTile*> GetAllTiles() const;

    // 반경 안 타일을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    TArray<UWorldTile*> GetTilesInRadius(FVector2D CenterHex, int32 Radius) const;

    // 총 타일 수를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Management")
    int32 GetTotalTileCount() const { return HexTiles.Num(); }

    // ========== hex 유틸 ==========

    // 인접 6칸 hex를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    TArray<FVector2D> GetHexNeighbors(FVector2D HexPosition) const;

    // 두 hex 사이 거리를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    int32 GetHexDistance(FVector2D Hex1, FVector2D Hex2) const;

    // 반경 안 hex 좌표를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    TArray<FVector2D> GetHexesInRadius(FVector2D CenterHex, int32 Radius) const;

    // hex를 월드 좌표로 바꿉니다.
    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    FVector HexToWorld(FVector2D HexPosition) const;

    // 월드 좌표를 hex로 바꿉니다.
    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    FVector2D WorldToHex(FVector WorldPosition) const;

    // 맵 안 유효한 hex인지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Hex Utilities")
    bool IsValidHexPosition(FVector2D HexPosition) const;

    // ========== 타일 호버 ==========

    // 타일 호버 시작을 처리합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Hover")
    void HandleTileHoverBegin(UWorldTile* HoveredTile);

    // 타일 호버 종료를 처리합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Hover")
    void HandleTileHoverEnd(UWorldTile* HoveredTile);

    // ========== 월드 설정 ==========

    // 월드 설정을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Settings")
    FWorldConfig GetWorldConfig() const { return WorldConfig; }

    // 월드 설정을 바꿉니다.
    UFUNCTION(BlueprintCallable, Category = "World Settings")
    void SetWorldConfig(const FWorldConfig& NewSettings);

    // 맵 반지름을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Settings")
    int32 GetMapRadius() const { return WorldConfig.WorldRadius; }

    // 타일 크기를 반환합니다. 고정값입니다.
    UFUNCTION(BlueprintCallable, Category = "World Settings")
    float GetTileSize() const { return TILE_SIZE; }

    // ========== 데이터 테이블 ==========

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void SetClimateDataTable(UDataTable* DataTable) { ClimateDataTable = DataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void SetLandTypeDataTable(UDataTable* DataTable) { LandTypeDataTable = DataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void SetBonusResourceDataTable(UDataTable* DataTable) { BonusResourceDataTable = DataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void SetStrategicResourceDataTable(UDataTable* DataTable) { StrategicResourceDataTable = DataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void SetLuxuryResourceDataTable(UDataTable* DataTable) { LuxuryResourceDataTable = DataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    UDataTable* GetClimateDataTable() const { return ClimateDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    UDataTable* GetLandTypeDataTable() const { return LandTypeDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    UDataTable* GetBonusResourceDataTable() const { return BonusResourceDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    UDataTable* GetStrategicResourceDataTable() const { return StrategicResourceDataTable; }

    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    UDataTable* GetLuxuryResourceDataTable() const { return LuxuryResourceDataTable; }

    // 데이터 테이블을 로드합니다.
    UFUNCTION(BlueprintCallable, Category = "Data Tables")
    void LoadDataTables();

	// ========== 도시 시작 위치 ==========

	// 육지·이웃 조건에 맞는 시작 도시 hex를 골라 StartingCityHexes에 넣습니다. 스폰은 하지 않습니다.
	UFUNCTION(BlueprintCallable, Category = "City Placement")
	void GenerateCities(int32 NumCities, int32 MinHexDistance = 5, int32 RequiredLandNeighbors = 5);

	UFUNCTION(BlueprintCallable, Category = "City Placement")
	TArray<FVector2D> GetStartingCityHexes() const { return StartingCityHexes; }

	// ========== 도시 조회 ==========

	UFUNCTION(BlueprintCallable, Category = "City Placement")
	int32 GetCityCount() const { return StartingCityHexes.Num(); }

	UFUNCTION(BlueprintCallable, Category = "City Placement")
	bool IsCityAtHex(FVector2D Hex) const { return StartingCityHexes.Contains(Hex); }

	// ========== 도시 관리 ==========

	// 데이터 배열에서 도시를 빼고 이벤트를 보냅니다. 액터 삭제는 WorldSpawner가 합니다.
	UFUNCTION(BlueprintCallable, Category = "City Placement")
	bool RemoveCityAt(FVector2D Hex);

    // ========== 산출 계산 ==========

    // 한 타일의 산출을 다시 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    void RecalculateTileYields(UWorldTile* Tile);

    // 모든 타일 산출을 다시 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    void RecalculateAllTileYields();

    // 기본 식량 산출을 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseFoodYield(UWorldTile* Tile) const;

    // 기본 생산력 산출을 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseProductionYield(UWorldTile* Tile) const;

    // 기본 골드 산출을 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseGoldYield(UWorldTile* Tile) const;

    // 기본 과학 산출을 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseScienceYield(UWorldTile* Tile) const;

    // 기본 신앙 산출을 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseFaithYield(UWorldTile* Tile) const;

    // 기본 이동 비용을 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateBaseMovementCost(UWorldTile* Tile) const;

    // 기본 전투 보너스를 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Tile Calculation")
    int32 CalculateCombatBonus(UWorldTile* Tile) const;

    // ========== 지형 생성 ==========

    // 땅·바다를 생성합니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateTerrain();

    // 자원을 생성합니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateResources();

    // 사치 자원을 뿌립니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateLuxuryResources(TArray<UWorldTile*>& LandTiles);

    // 전략 자원을 뿌립니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateStrategicResources(TArray<UWorldTile*>& LandTiles);

    // 보너스 자원을 뿌립니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateBonusResources(TArray<UWorldTile*>& LandTiles);

    // 기후대를 생성합니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateClimateZones();

    // 땅 타입을 생성합니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateLandTypes();

    // 숲을 생성합니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GenerateForests();

    // 판게아 스타일 지형을 생성합니다.
    UFUNCTION(BlueprintCallable, Category = "World Generation")
    void GeneratePangaeaTerrain();

    // ========== 통계 ==========

    // 땅 타일 수를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetLandTileCount() const;

    // 바다 타일 수를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetOceanTileCount() const;

    // 해당 자원 카테고리 타일 수를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetResourceTileCount(EResourceCategory ResourceCategory) const;

    // 해당 기후 타일 수를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetClimateTileCount(EClimateType ClimateType) const;

    // 해당 땅 타입 타일 수를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetLandTypeTileCount(ELandType LandType) const;

    // 숲이 있는 타일 수를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "World Statistics")
    int32 GetForestTileCount() const;

private:
    // ========== 내부 ==========

    // hex 맵을 비우고 새로 만듭니다.
    void InitializeHexTiles();

    // 빈 타일 오브젝트를 만듭니다.
    UWorldTile* CreateNewTile(FVector2D HexPosition);

    // 타일 오브젝트를 파괴합니다.
    void DestroyTile(UWorldTile* Tile);

    // 기후·지형·숲이 자원과 맞는지 반환합니다.
    bool IsResourceCompatibleWithTile(const TArray<EClimateType>& CompatibleClimates, const TArray<ELandType>& CompatibleLandTypes, bool bRequiresForest, UWorldTile* Tile) const;
};
