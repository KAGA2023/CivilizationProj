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

    // ========== 생성자 ==========
    FAIPlayerStruct()
    {
        PlayerIndex = -1;
        CurrentState = EAITurnState::Idle;
        bIsTurnActive = false;
        CurrentTurnRound = 0;
        PendingUnitMovements = 0;
        PendingCombatActions = 0;
    }
};

