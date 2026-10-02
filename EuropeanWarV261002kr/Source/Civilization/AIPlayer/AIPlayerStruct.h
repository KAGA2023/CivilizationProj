// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIPlayerStruct.generated.h"

class ASuperPlayerState;
class AUnitCharacterBase;

UENUM(BlueprintType)
enum class EAITurnState : uint8
{
    None                        UMETA(DisplayName = "None"),                           // 없음 (초기값)
    Idle                        UMETA(DisplayName = "Idle"),                           // 대기
    ProcessingDiplomacy         UMETA(DisplayName = "Processing Diplomacy"),           // 외교 처리
    ProcessingResearch          UMETA(DisplayName = "Processing Research"),            // 연구 처리
    ProcessingCityProduction    UMETA(DisplayName = "Processing City Production"),     // 도시 생산 처리
    ProcessingTilePurchase      UMETA(DisplayName = "Processing Tile Purchase"),       // 타일 구매 처리
    ProcessingFacility          UMETA(DisplayName = "Processing Facility"),            // 시설 건설 처리
    ProcessingBuilderMovement   UMETA(DisplayName = "Processing Builder Movement"),     // 건설자 이동 처리
    ProcessingBuilderBuild      UMETA(DisplayName = "Processing Builder Build"),        // 건설자 시설 건설 처리
    ProcessingCombatUnitMovement UMETA(DisplayName = "Processing Combat Unit Movement"), // 병사 유닛 이동 처리
    ProcessingCombatUnitCombat  UMETA(DisplayName = "Processing Combat Unit Combat"),    // 병사 유닛 전투 처리
    WaitingForAsync             UMETA(DisplayName = "Waiting For Async"),              // 비동기 대기
    TurnComplete                UMETA(DisplayName = "Turn Complete")                   // 턴 완료
};

UENUM(BlueprintType)
enum class EAILastProductionType : uint8
{
    None                        UMETA(DisplayName = "None"),                          // 없음 (초기값)
    Builder                     UMETA(DisplayName = "Builder"),                        // 건설자
    Combat                      UMETA(DisplayName = "Combat"),                         // 병사
    Building                    UMETA(DisplayName = "Building")                        // 건물
};

// 한 AI 슬롯의 턴 상태머신 데이터
USTRUCT(BlueprintType)
struct CIVILIZATION_API FAIPlayerStruct
{
    GENERATED_BODY()

    // ========== 기본 식별 ==========

    // 문명 슬롯. 0=호스트/싱글, 1=1v1 참가자, 나머지는 AI
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Info")
    int32 PlayerIndex = -1;

    // 이 슬롯의 PlayerState. 약한 참조로 수명을 안전하게 둡니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Info")
    TWeakObjectPtr<ASuperPlayerState> PlayerStateRef;

    // ========== 상태머신 ==========

    // 지금 처리 중인 턴 단계
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine")
    EAITurnState CurrentState = EAITurnState::Idle;

    // ========== 턴 진행 ==========

    // 지금 이 AI의 턴이 진행 중인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn State")
    bool bIsTurnActive = false;

    // 지금 처리 중인 라운드 번호
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn State")
    int32 CurrentTurnRound = 0;

    // ========== 비동기 추적 ==========

    // 아직 끝나지 않은 유닛 이동 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Async")
    int32 PendingUnitMovements = 0;

    // 아직 끝나지 않은 전투 연출 수
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Async")
    int32 PendingCombatActions = 0;

    // ========== 생산 순서 ==========

    // 마지막 생산 종류. 건설자 → 병사 → 건물 순서를 기억합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Order")
    EAILastProductionType LastProductionType = EAILastProductionType::None;

    // ========== 시설 목표 ==========

    // 건설자가 갈 목표 타일. (-1, -1)이면 아직 없습니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility")
    FVector2D TargetFacilityTile = FVector2D(-1, -1);

    // true면 목표 타일에서 수리, false면 건설합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility")
    bool bTargetFacilityIsRepair = false;

    // ========== 비동기 이후 상태 ==========

    // 비동기 작업이 끝나면 갈 상태. None이면 턴을 끝냅니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine")
    EAITurnState PendingPostAsyncState = EAITurnState::None;

    // ========== 병사 순차 처리 ==========

    // 이번 턴에 처리할 병사 유닛 큐
    UPROPERTY()
    TArray<TWeakObjectPtr<AUnitCharacterBase>> CombatUnitsQueue;

    // 지금 처리 중인 병사 인덱스
    UPROPERTY()
    int32 CurrentCombatUnitIndex = 0;

    // 지금 한 병사를 처리 중인지 여부
    UPROPERTY()
    bool bIsProcessingCombatUnit = false;

    FAIPlayerStruct()
    {
        PlayerIndex = -1;
        CurrentState = EAITurnState::Idle;
        bIsTurnActive = false;
        CurrentTurnRound = 0;
        PendingUnitMovements = 0;
        PendingCombatActions = 0;
        LastProductionType = EAILastProductionType::None;
        TargetFacilityTile = FVector2D(-1, -1);
        bTargetFacilityIsRepair = false;
        PendingPostAsyncState = EAITurnState::None;
        CurrentCombatUnitIndex = 0;
        bIsProcessingCombatUnit = false;
    }
};

// 타일 구매 후보를 총 산출량 순으로 정렬할 때 씁니다.
USTRUCT(BlueprintType)
struct CIVILIZATION_API FTileWithTotalYield
{
    GENERATED_BODY()

    // 타일 hex 좌표
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Purchase")
    FVector2D Coordinate;

    // 생산량+식량+과학+골드를 더한 값
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Purchase")
    int32 TotalYield = 0;

    FTileWithTotalYield()
    {
        Coordinate = FVector2D::ZeroVector;
        TotalYield = 0;
    }

    FTileWithTotalYield(FVector2D InCoord, int32 InTotalYield)
        : Coordinate(InCoord), TotalYield(InTotalYield)
    {}

    // 총 산출량이 큰 쪽이 앞에 오도록 비교합니다.
    static bool CompareDescending(const FTileWithTotalYield& A, const FTileWithTotalYield& B)
    {
        return A.TotalYield > B.TotalYield;
    }
};
