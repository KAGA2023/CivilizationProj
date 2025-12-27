// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AIPlayerStruct.generated.h"

class ASuperPlayerState;

UENUM(BlueprintType)
enum class EAITurnState : uint8
{
    Idle                        UMETA(DisplayName = "Idle"),                           // 대기
    ProcessingDiplomacy         UMETA(DisplayName = "Processing Diplomacy"),           // 외교 처리
    ProcessingResearch          UMETA(DisplayName = "Processing Research"),            // 연구 처리
    ProcessingCityProduction    UMETA(DisplayName = "Processing City Production"),     // 도시 생산 처리
    ProcessingTilePurchase      UMETA(DisplayName = "Processing Tile Purchase"),       // 타일 구매 처리
    ProcessingFacility          UMETA(DisplayName = "Processing Facility"),            // 시설 건설 처리
    ProcessingUnitMovement      UMETA(DisplayName = "Processing Unit Movement"),       // 유닛 이동 처리
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

USTRUCT(BlueprintType)
struct CIVILIZATION_API FAIPlayerStruct
{
    GENERATED_BODY()

    // ========== 기본 식별 정보 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Info")
    int32 PlayerIndex = -1; // 플레이어 인덱스 (1~3: AI 플레이어)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI Info")
    TWeakObjectPtr<ASuperPlayerState> PlayerStateRef; // PlayerState 참조 (약한 참조로 메모리 안전성 확보)

    // ========== 상태머신 상태 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "State Machine")
    EAITurnState CurrentState = EAITurnState::Idle; // 현재 상태

    // ========== 턴 진행 상태 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn State")
    bool bIsTurnActive = false; // 현재 턴 진행 중인지 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Turn State")
    int32 CurrentTurnRound = 0; // 현재 처리 중인 라운드

    // ========== 애니메이션 액션 추적 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Async")
    int32 PendingUnitMovements = 0; // 대기 중인 유닛 이동 수

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Async")
    int32 PendingCombatActions = 0; // 대기 중인 전투 액션 수

    // ========== 생산 순서 추적 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production Order")
    EAILastProductionType LastProductionType = EAILastProductionType::None; // 마지막 생산 타입 (건설자 -> 병사 -> 건물 순서)

    // ========== 시설 건설 목표 추적 ==========
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility")
    FVector2D TargetFacilityTile = FVector2D(-1, -1); // 건설자가 이동할 목표 타일 위치 (-1, -1이면 미설정)

    // ========== 생성자 ==========
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
    }
};

// ========== 타일 구매 정렬용 구조체 ==========
USTRUCT(BlueprintType)
struct CIVILIZATION_API FTileWithTotalYield
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Purchase")
    FVector2D Coordinate; // 타일 좌표

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Purchase")
    int32 TotalYield = 0; // 생산량 + 식량 + 과학력 + 골드 총합

    // ========== 생성자 ==========
    FTileWithTotalYield()
    {
        Coordinate = FVector2D::ZeroVector;
        TotalYield = 0;
    }

    FTileWithTotalYield(FVector2D InCoord, int32 InTotalYield)
        : Coordinate(InCoord), TotalYield(InTotalYield)
    {}

    // ========== 정렬을 위한 비교 함수 ==========
    static bool CompareDescending(const FTileWithTotalYield& A, const FTileWithTotalYield& B)
    {
        return A.TotalYield > B.TotalYield;
    }
};

