// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitManager.generated.h"

// 전투 실행 완료 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCombatExecuted);

class UWorldComponent;
class AUnitCharacterBase;
class USuperGameInstance;

// 전투 결과 구조체 전방 선언
struct FCombatResult;

// A* 알고리즘용 노드 구조체
USTRUCT(BlueprintType)
struct CIVILIZATION_API FAStarNode
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector2D HexPosition = FVector2D::ZeroVector; // 육각형 좌표

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 GCost = 0; // 시작점으로부터의 실제 비용

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 HCost = 0; // 목표점까지의 휴리스틱 비용

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    int32 FCost = 0; // 총 비용 (G + H)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    FVector2D ParentHex = FVector2D::ZeroVector; // 부모 노드 좌표
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Pathfinding")
    bool bIsWalkable = true; // 이동 가능한지 여부

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

    // 우선순위 큐를 위한 비교 연산자 (F값이 작을수록 우선순위 높음)
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

    // 유닛 소환 관련 함수들
    UFUNCTION(BlueprintCallable, Category = "Unit Spawning")
    class AUnitCharacterBase* SpawnUnitAtHex(FVector2D HexPosition, const FName& RowName, int32 PlayerIndex = -1); // 지정된 육각형 좌표에 유닛 소환 (PlayerIndex로 소유 플레이어 지정)

    // 유닛 관리 함수들
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    TArray<class AUnitCharacterBase*> GetAllUnits() const; // 모든 유닛 가져오기

    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void DestroyUnit(class AUnitCharacterBase* Unit, FVector2D HexPosition); // 유닛 완전 제거, 죽을때 사용 (HexToUnitMap에서 먼저 제거 후 Destroy)

    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void ClearAllUnits(); // 모든 유닛 제거

    // 월드 컴포넌트 설정
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void SetWorldComponent(class UWorldComponent* WorldComponent); // 월드 컴포넌트 설정

    // 유닛 위치 관리 함수들
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    class AUnitCharacterBase* GetUnitAtHex(FVector2D HexPosition) const; // 육각형 좌표의 유닛 가져오기

    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    bool SetUnitAtHex(FVector2D HexPosition, class AUnitCharacterBase* Unit); // 육각형 좌표에 유닛 설정

    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    void RemoveUnitFromHex(FVector2D HexPosition); // 육각형 좌표에서 유닛 제거, 이동할때 사용, 죽을때 사용하지 말것

    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    bool CanPlaceUnitAtHex(FVector2D HexPosition) const; // 해당 위치에 유닛 배치 가능 여부

    // 유닛 이동력 관리
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    int32 GetUnitRemainingMovement(class AUnitCharacterBase* Unit) const; // 유닛의 남은 이동력 가져오기

    // 건설자 판단
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    bool IsBuilderUnit(class AUnitCharacterBase* Unit) const; // 건설자 유닛인지 확인

    // 전투 유닛 판단
    UFUNCTION(BlueprintCallable, Category = "Unit Management")
    bool IsCombatUnit(class AUnitCharacterBase* Unit) const; // 전투 유닛인지 확인

    // 이동 선택 시스템
    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    void HandleMoveSelection(class UWorldTile* ClickedTile); // 이동용 2단계 타일 클릭 처리

    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    void ClearMoveSelection(); // 이동 선택 초기화

    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    class UWorldTile* GetMoveFirstSelectedTile() const { return MoveFirstSelectedTile; } // 첫 번째 선택된 이동 타일 가져오기

    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    class UWorldTile* GetMoveSecondSelectedTile() const { return MoveSecondSelectedTile; } // 두 번째 선택된 이동 타일 가져오기

    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    bool HasMoveFirstSelection() const { return MoveFirstSelectedTile != nullptr; } // 첫 번째 이동 선택 여부

    UFUNCTION(BlueprintCallable, Category = "Move Selection")
    bool HasMoveSecondSelection() const { return MoveSecondSelectedTile != nullptr; } // 두 번째 이동 선택 여부

    // 내부 유닛 이동 함수
    void MoveUnitFromFirstToSecondSelection(); // 첫 번째 선택에서 두 번째 선택으로 유닛 이동

    // 시각적 이동 애니메이션 함수들
    void StartVisualMovement(class AUnitCharacterBase* Unit, const TArray<FVector2D>& Path); // 시각적 이동 시작
    void CompleteMovement(); // 이동 완료
    
    // AIController로부터 이동 완료 알림을 받는 함수
    UFUNCTION()
    void OnUnitMovementComplete(class AUnitCharacterBase* Unit, FVector2D FinalHex);
    

    // 경로 찾기 및 이동 시스템
    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector2D> FindPath(FVector2D StartHex, FVector2D EndHex) const; // A* 알고리즘을 사용한 최적 경로 찾기

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector2D> FindPathWithMovementCost(FVector2D StartHex, FVector2D EndHex, int32 MaxMovementCost) const; // 최대 이동 비용 제한이 있는 경로 찾기

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    bool CanMoveToHex(FVector2D HexPosition) const; // 해당 육각형으로 이동 가능한지 확인

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 CalculateHeuristic(FVector2D StartHex, FVector2D EndHex) const; // 휴리스틱 함수 (육각형 거리 기반)

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    TArray<FVector2D> ReconstructPath(const TMap<FVector2D, FAStarNode>& CameFrom, FVector2D Current) const; // 경로 재구성

    UFUNCTION(BlueprintCallable, Category = "Pathfinding")
    int32 GetMovementCostBetweenHexes(FVector2D FromHex, FVector2D ToHex) const; // 두 육각형 간의 이동 비용 계산

    // 층수 시스템 관련 함수들
    UFUNCTION(BlueprintCallable, Category = "Floor System")
    int32 GetFloorLevel(ELandType LandType) const; // 지형 타입을 층수로 변환

    UFUNCTION(BlueprintCallable, Category = "Floor System")
    bool CanMoveBetweenHexes(FVector2D FromHex, FVector2D ToHex) const; // 두 육각형 간 층수 이동 가능성 체크

    UFUNCTION(BlueprintCallable, Category = "Floor System")
    int32 GetMovementCostBetweenHexesWithFloor(FVector2D FromHex, FVector2D ToHex) const; // 층수를 고려한 이동 비용 계산

    // 전투 선택 시스템
    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    void HandleCombatSelection(class UWorldTile* ClickedTile); // 전투용 2단계 타일 클릭 처리

    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    void ClearCombatSelection(); // 전투 선택 초기화

    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    class UWorldTile* GetCombatFirstSelectedTile() const { return CombatFirstSelectedTile; } // 첫 번째 선택된 전투 타일 가져오기

    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    class UWorldTile* GetCombatSecondSelectedTile() const { return CombatSecondSelectedTile; } // 두 번째 선택된 전투 타일 가져오기

    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    bool HasCombatFirstSelection() const { return CombatFirstSelectedTile != nullptr; } // 첫 번째 전투 선택 여부

    UFUNCTION(BlueprintCallable, Category = "Combat Selection")
    bool HasCombatSecondSelection() const { return CombatSecondSelectedTile != nullptr; } // 두 번째 전투 선택 여부

    // 전투 실행 함수
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void ExecuteCombatBetweenSelectedUnits(); // 선택된 유닛들 간 전투 실행

    // 전투 시각화 완료 콜백 (AIController에서 호출)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    void OnCombatVisualizationComplete(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, const struct FCombatResult& CombatResult, FVector2D AttackerHex, FVector2D DefenderHex);

    // 전투 실행 완료 이벤트
    UPROPERTY(BlueprintAssignable, Category = "Combat Events")
    FOnCombatExecuted OnCombatExecuted;

private:
    // 월드 컴포넌트 참조
    UPROPERTY()
    class UWorldComponent* WorldComponent = nullptr;

    // 소환된 유닛들 관리
    UPROPERTY()
    TArray<class AUnitCharacterBase*> SpawnedUnits;

    // 유닛 위치 추적 (WorldComponent에서 이동)
    UPROPERTY()
    TMap<FVector2D, class AUnitCharacterBase*> HexToUnitMap;

    // 이동용 2단계 선택 시스템
    UPROPERTY()
    class UWorldTile* MoveFirstSelectedTile = nullptr; // 첫 번째 선택된 이동 유닛 타일

    UPROPERTY()
    class UWorldTile* MoveSecondSelectedTile = nullptr; // 두 번째 선택된 이동 유닛 타일

    // 전투용 2단계 선택 시스템
    UPROPERTY()
    class UWorldTile* CombatFirstSelectedTile = nullptr; // 첫 번째 선택된 전투 유닛 타일

    UPROPERTY()
    class UWorldTile* CombatSecondSelectedTile = nullptr; // 두 번째 선택된 전투 유닛 타일

};
