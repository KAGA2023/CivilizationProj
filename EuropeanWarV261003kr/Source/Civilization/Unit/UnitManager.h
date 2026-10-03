// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitManager.generated.h"

// 전투 계산이 끝나면 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatExecuted);

// 선택 유닛이 바뀌면 브로드캐스트합니다. nullptr이면 선택 해제입니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectedUnitChanged, AUnitCharacterBase*, NewSelectedUnit);

class UWorldComponent;
class AUnitCharacterBase;
class USuperGameInstance;

// 전투 결과 구조체 전방 선언
struct FCombatResult;

// A* 경로 탐색용 한 칸
USTRUCT(BlueprintType)
struct CIVILIZATION_API FAStarNode
{
    GENERATED_BODY()

    // 이 노드의 hex 좌표
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector2D HexPosition = FVector2D::ZeroVector;

    // 시작점에서 여기까지 실제 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 GCost = 0;

    // 여기부터 목표까지 휴리스틱 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 HCost = 0;

    // G+H 총 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 FCost = 0;

    // 경로를 되돌릴 때 쓰는 부모 hex
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector2D ParentHex = FVector2D::ZeroVector;
    
    // 이 칸으로 들어갈 수 있는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsWalkable = true;

    FAStarNode()
    {
        HexPosition = FVector2D::ZeroVector;
        GCost = 0;
        HCost = 0;
        FCost = 0;
        ParentHex = FVector2D::ZeroVector;
        bIsWalkable = true;
    }

    FAStarNode(FVector2D InHexPosition, int32 InGCost, int32 InHCost, FVector2D InParentHex, bool InIsWalkable)
    {
        HexPosition = InHexPosition;
        GCost = InGCost;
        HCost = InHCost;
        FCost = GCost + HCost;
        ParentHex = InParentHex;
        bIsWalkable = InIsWalkable;
    }

    // F가 작을수록 우선순위가 높습니다.
    bool operator>(const FAStarNode& Other) const
    {
        return FCost > Other.FCost;
    }

    bool operator<(const FAStarNode& Other) const
    {
        return FCost < Other.FCost;
    }
};

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CIVILIZATION_API UUnitManager : public UActorComponent
{
    GENERATED_BODY()

public:
    UUnitManager();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========== 스폰 ==========

    // 해당 hex에 유닛을 스폰합니다. PlayerIndex는 소유 슬롯입니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Spawning")
    class AUnitCharacterBase* SpawnUnitAtHex(FVector2D HexPosition, const FName& RowName, int32 PlayerIndex = -1, bool bSkipPlacementCheck = false);

    // 도시 주변 1칸에서 스폰 가능한 타일을 고릅니다. 없으면 (-1, -1)입니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Spawning")
    FVector2D FindSpawnLocationNearCity(FVector2D CityHex) const;

    // ========== 유닛 관리 ==========

    // 스폰된 모든 유닛을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    TArray<class AUnitCharacterBase*> GetAllUnits() const;

    // HexToUnitMap에서 뺀 뒤 유닛을 파괴합니다. 사망 때 씁니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void DestroyUnit(class AUnitCharacterBase* Unit, FVector2D HexPosition);

    // 모든 유닛을 제거합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void ClearAllUnits();

    // 월드 컴포넌트를 연결합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void SetWorldComponent(class UWorldComponent* WorldComponent);

    // 해당 hex의 유닛을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    class AUnitCharacterBase* GetUnitAtHex(FVector2D HexPosition) const;

    // 해당 hex에 유닛을 올립니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    bool SetUnitAtHex(FVector2D HexPosition, class AUnitCharacterBase* Unit);

    // 유닛이 있는 hex를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    FVector2D GetHexPositionForUnit(class AUnitCharacterBase* Unit) const;

    // 해당 hex에서 유닛만 뺍니다. 이동 때 쓰고 사망 때는 쓰지 않습니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void RemoveUnitFromHex(FVector2D HexPosition);

    // 해당 위치에 유닛을 둘 수 있는지 반환합니다. MovingUnit의 예약은 무시합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    bool CanPlaceUnitAtHex(FVector2D HexPosition, class AUnitCharacterBase* MovingUnit = nullptr) const;

    // 타국 영토에 남은 유닛을 가장 가까운 배치 가능 타일로 순간이동합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    bool EvictUnitToNearestValidTile(class AUnitCharacterBase* Unit);

    // 두 슬롯 사이 타국 영토에 남은 유닛을 모두 쫓아냅니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void EvictUnitsOnForeignTilesBetweenPlayers(int32 PlayerA, int32 PlayerB);

    // 인접 6칸을 모두 갈 수 없으면 갇힌 유닛으로 봅니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    bool IsUnitTrapped(class AUnitCharacterBase* Unit) const;

    // 유닛의 남은 이동력을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    int32 GetUnitRemainingMovement(class AUnitCharacterBase* Unit) const;

    // 건설자인지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    bool IsBuilderUnit(class AUnitCharacterBase* Unit) const;

    // ========== 건설자 액션 ==========

    // 건설 몽타주가 끝나면 시설을 짓고 건설자를 제거합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void RequestBuilderBuildFacility(FVector2D Hex, FName FacilityRowName);

    // 수리 몽타주가 끝나면 시설을 수리합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void RequestBuilderRepairFacility(FVector2D Hex);

    // 철거 몽타주가 끝나면 시설을 제거합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void RequestBuilderDestroyFacility(FVector2D Hex);

    // 전투 유닛인지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    bool IsCombatUnit(class AUnitCharacterBase* Unit) const;

    // ========== 이동 범위 ==========

    // BFS로 1턴 안에 갈 수 있는 타일을 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Movement Range")
    TArray<FVector2D> CalculateMovementRange(class AUnitCharacterBase* Unit) const;

    // 이동 가능 타일은 밝게, 나머지는 어둡게 표시합니다.
    UFUNCTION(BlueprintCallable, Category = "Movement Range")
    void ShowMovementRangeWithBrightness(class AUnitCharacterBase* Unit);

    // 모든 타일 밝기를 Stencil 0으로 되돌립니다.
    UFUNCTION(BlueprintCallable, Category = "Movement Range")
    void ClearAllTileBrightness();

    // ========== 이동 선택 ==========

    // 이동용 2단계 타일 클릭을 처리합니다.
    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    void HandleMoveSelection(class UWorldTile* ClickedTile);

    // 이동 선택을 지웁니다.
    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    void ClearMoveSelection();

    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    class UWorldTile* GetMoveFirstSelectedTile() const { return MoveFirstSelectedTile; }

    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    class UWorldTile* GetMoveSecondSelectedTile() const { return MoveSecondSelectedTile; }

    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    bool HasMoveFirstSelection() const { return MoveFirstSelectedTile != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    bool HasMoveSecondSelection() const { return MoveSecondSelectedTile != nullptr; }

    // 두 번째 클릭이 이동 확정이면 칸을 돌려주고 선택과 하이라이트를 지웁니다.
    bool ConfirmMoveSelection(class UWorldTile* ClickedTile, FVector2D& OutFromHex, FVector2D& OutToHex);

    // 출발 칸의 유닛을 도착 칸까지 옮깁니다. 성공하면 true입니다.
    bool MoveUnitFromHexToHex(FVector2D FromHex, FVector2D ToHex);

    // 그 슬롯 유닛의 이동력과 공격 가능 상태를 되돌립니다.
    void ResetPlayerUnitTurn(int32 PlayerIndex);

    // ========== 시각적 이동 ==========

    // 경로를 따라 시각적 이동을 시작합니다. 실제로 출발하면 true입니다.
    bool StartVisualMovement(class AUnitCharacterBase* Unit, const TArray<FVector2D>& Path);

    // 연출 없이 바로 옮깁니다. AI용입니다.
    void StartMovementImmediate(class AUnitCharacterBase* Unit, const TArray<FVector2D>& Path);

    // 이동을 끝냅니다.
    void CompleteMovement();
    
    // AIController가 이동 완료를 알릴 때 호출합니다.
    UFUNCTION()
    void OnUnitMovementComplete(class AUnitCharacterBase* Unit, FVector2D FinalHex);
    

    // ========== 경로 찾기 ==========

    // A*로 최적 경로를 찾습니다. 국경선을 적용합니다.
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector2D> FindPath(FVector2D StartHex, FVector2D EndHex, int32 MoverPlayerIndex = 0) const;

    // 최대 이동 비용 안에서 경로를 찾습니다.
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector2D> FindPathWithMovementCost(FVector2D StartHex, FVector2D EndHex, int32 MaxMovementCost, int32 MoverPlayerIndex = 0) const;

    // 본인·무주·전쟁·동맹 타일만 들어갈 수 있는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool CanMoveToHex(FVector2D HexPosition, int32 MoverPlayerIndex = 0) const;

    // 두 hex 사이 휴리스틱(육각 거리)을 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 CalculateHeuristic(FVector2D StartHex, FVector2D EndHex) const;

    // CameFrom 맵으로 경로를 되돌립니다.
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector2D> ReconstructPath(const TMap<FVector2D, FAStarNode>& CameFrom, FVector2D Current) const;

    // 두 hex 사이 이동 비용을 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 GetMovementCostBetweenHexes(FVector2D FromHex, FVector2D ToHex, int32 MoverPlayerIndex = 0) const;

    // ========== 층수 ==========

    // 지형 타입을 층수로 바꿉니다.
    UFUNCTION(BlueprintCallable, Category = "Floor System")
    int32 GetFloorLevel(ELandType LandType) const;

    // 층수·국경을 보고 두 hex 사이 이동이 가능한지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Floor System")
    bool CanMoveBetweenHexes(FVector2D FromHex, FVector2D ToHex, int32 MoverPlayerIndex = 0) const;

    // 층수를 반영한 이동 비용을 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Floor System")
    int32 GetMovementCostBetweenHexesWithFloor(FVector2D FromHex, FVector2D ToHex, int32 MoverPlayerIndex = 0) const;

    // ========== 전투 선택 ==========

    // 전투용 2단계 타일 클릭을 처리합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    void HandleCombatSelection(class UWorldTile* ClickedTile);

    // 전투 선택을 지웁니다.
    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    void ClearCombatSelection();

    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    class UWorldTile* GetCombatFirstSelectedTile() const { return CombatFirstSelectedTile; }

    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    class UWorldTile* GetCombatSecondSelectedTile() const { return CombatSecondSelectedTile; }

    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    bool HasCombatFirstSelection() const { return CombatFirstSelectedTile != nullptr; }

    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    bool HasCombatSecondSelection() const { return CombatSecondSelectedTile != nullptr; }

    // ========== 전투 실행 ==========

    // 두 번째 클릭이 공격 확정이면 칸을 돌려주고 선택과 하이라이트를 지웁니다.
    bool ConfirmCombatSelection(class UWorldTile* ClickedTile, FVector2D& OutAttackerHex, FVector2D& OutTargetHex);

    // 선택된 유닛끼리 전투를 실행합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteCombatBetweenSelectedUnits();

    // 공격자 칸과 대상 칸으로 전투를 실행합니다. 외교 검사는 호스트 확정에서만 켭니다.
    bool CombatFromHexToHex(FVector2D AttackerHex, FVector2D TargetHex, bool bCheckDiplomacy = true);

    // 전투 연출이 끝나면 AIController가 호출합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnCombatVisualizationComplete(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, const struct FCombatResult& CombatResult, FVector2D AttackerHex, FVector2D DefenderHex);

    // 유닛 HP바를 갱신합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void UpdateUnitHPBar(AUnitCharacterBase* Unit);

    // 전투 계산이 끝나면 브로드캐스트합니다.
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatExecuted OnCombatExecuted;

    // 선택 유닛이 바뀌면 브로드캐스트합니다. UnitInfoUI 표시에 씁니다.
    UPROPERTY(BlueprintAssignable, Category = "Unit Selection")
    FOnSelectedUnitChanged OnSelectedUnitChanged;

private:
    // ========== 내부 데이터 ==========

    // 타일·거리 계산용 월드 컴포넌트
    UPROPERTY()
    class UWorldComponent* WorldComponent = nullptr;

    // 스폰된 유닛 목록
    UPROPERTY()
    TArray<class AUnitCharacterBase*> SpawnedUnits;

    // hex → 그 칸의 유닛
    UPROPERTY()
    TMap<FVector2D, class AUnitCharacterBase*> HexToUnitMap;

    // 이동용 첫 클릭 타일
    UPROPERTY()
    class UWorldTile* MoveFirstSelectedTile = nullptr;

    // 이동용 둘째 클릭 타일
    UPROPERTY()
    class UWorldTile* MoveSecondSelectedTile = nullptr;

    // 전투용 첫 클릭 타일
    UPROPERTY()
    class UWorldTile* CombatFirstSelectedTile = nullptr;

    // 전투용 둘째 클릭 타일
    UPROPERTY()
    class UWorldTile* CombatSecondSelectedTile = nullptr;

    // 지금 선택된 유닛
    UPROPERTY()
    class AUnitCharacterBase* CurrentSelectedUnit = nullptr;

    // 밝기를 바꾼 타일 액터
    UPROPERTY()
    TArray<class AWorldTileActor*> BrightnessAffectedTiles;

    // 선택 유닛이 1턴 안에 갈 수 있는 타일
    UPROPERTY()
    TArray<FVector2D> CurrentReachableTiles;
    
    // 빨간 외곽선이 켜진 적 유닛
    UPROPERTY()
    TArray<class AUnitCharacterBase*> HighlightedEnemyUnits;

    // 무주·본인·전쟁·동맹 타일만 들어갈 수 있는지 반환합니다.
    bool CanPlayerEnterTile(FVector2D HexPosition, int32 MoverPlayerIndex) const;

    // 공격자 슬롯이 그 칸의 유닛이나 도시와 전쟁 중인지 반환합니다.
    bool IsHostileCombatTarget(class AUnitCharacterBase* Attacker, FVector2D TargetHex) const;

    // 쫓아내기용으로 가장 가까운 배치 가능 hex를 찾습니다.
    bool FindNearestValidTileForEviction(FVector2D FromHex, class AUnitCharacterBase* Unit, FVector2D& OutHex) const;

public:
    // ========== 유닛 선택 ==========

    // 선택 유닛을 바꾸고 외곽선을 맞춥니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void SetSelectedUnit(class AUnitCharacterBase* Unit);

    // 지금 선택된 유닛을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    class AUnitCharacterBase* GetSelectedUnit() const { return CurrentSelectedUnit; }

    // 유닛 선택을 해제합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void ClearSelectedUnit();

    // 사람 슬롯용으로 도달 가능 타일을 계산해 저장합니다.
    UFUNCTION(BlueprintCallable, Category = "Movement Range")
    void CalculateAndStoreReachableTiles(class AUnitCharacterBase* Unit);

    // 해당 타일이 도달 가능 목록에 있는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Movement Range")
    bool IsReachableTile(FVector2D TilePos) const;
    
    // ========== 공격 범위 ==========

    // 공격 가능한 적 유닛을 찾습니다.
    UFUNCTION(BlueprintCallable, Category = "Attack Range")
    TArray<AUnitCharacterBase*> CalculateAttackableEnemies(class AUnitCharacterBase* Unit) const;
    
    // 공격 가능 적에게 빨간 외곽선을 켭니다.
    UFUNCTION(BlueprintCallable, Category = "Attack Range")
    void ShowAttackableEnemies(class AUnitCharacterBase* Unit);
    
    // 적 외곽선을 끕니다.
    UFUNCTION(BlueprintCallable, Category = "Attack Range")
    void ClearAttackableEnemies();
};
