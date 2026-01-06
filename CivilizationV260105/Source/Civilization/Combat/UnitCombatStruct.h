// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnitCombatStruct.generated.h"

// 전투 결과 구조체
USTRUCT(BlueprintType)
struct FCombatResult
{
    GENERATED_BODY()

    // 생존 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    bool bAttackerAlive = true;              // 공격자 생존 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    bool bDefenderAlive = true;               // 피격자 생존 여부

    // 데미지 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    int32 AttackerDamageDealt = 0;            // 공격자가 준 데미지

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    int32 DefenderDamageDealt = 0;           // 피격자가 반격으로 준 데미지

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    int32 AttackerDamageTaken = 0;            // 공격자가 받은 데미지 (반격 데미지)

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    int32 DefenderDamageTaken = 0;            // 피격자가 받은 데미지 (공격 데미지)

    // 기본 생성자
    FCombatResult()
        : bAttackerAlive(true)
        , bDefenderAlive(true)
        , AttackerDamageDealt(0)
        , DefenderDamageDealt(0)
        , AttackerDamageTaken(0)
        , DefenderDamageTaken(0)
    {}

    // 전체 초기화 생성자
    FCombatResult(
        bool InAttackerAlive,
        bool InDefenderAlive,
        int32 InAttackerDamageDealt,
        int32 InDefenderDamageDealt,
        int32 InAttackerDamageTaken,
        int32 InDefenderDamageTaken
    )
        : bAttackerAlive(InAttackerAlive)
        , bDefenderAlive(InDefenderAlive)
        , AttackerDamageDealt(InAttackerDamageDealt)
        , DefenderDamageDealt(InDefenderDamageDealt)
        , AttackerDamageTaken(InAttackerDamageTaken)
        , DefenderDamageTaken(InDefenderDamageTaken)
    {}

    // 결과 초기화 함수
    void Reset()
    {
        bAttackerAlive = true;
        bDefenderAlive = true;
        AttackerDamageDealt = 0;
        DefenderDamageDealt = 0;
        AttackerDamageTaken = 0;
        DefenderDamageTaken = 0;
    }

    // 유효한 전투 결과인지 확인
    bool IsValid() const
    {
        return AttackerDamageDealt >= 0 && 
               DefenderDamageDealt >= 0 && 
               AttackerDamageTaken >= 0 && 
               DefenderDamageTaken >= 0;
    }
};

// 전투 시각화 데이터 구조체
USTRUCT(BlueprintType)
struct FCombatVisualizationData
{
    GENERATED_BODY()

    // 전투 참여 유닛
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Visualization")
    class AUnitCharacterBase* Attacker = nullptr;  // 공격자 참조

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Visualization")
    class AUnitCharacterBase* Defender = nullptr;  // 방어자 참조

    // 전투 결과
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Visualization")
    FCombatResult CombatResult;  // 전투 결과

    // 공격자 원래 위치 (복귀용)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Visualization")
    FVector2D AttackerOriginalHexPosition = FVector2D::ZeroVector;  // 공격자 원래 위치

    // 전투 사거리 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Visualization")
    int32 CombatRange = 0;  // 현재 전투의 사거리

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Visualization")
    bool bIsRangedAttack = false;  // 원거리 공격 여부 (Range > 1)

    // 기본 생성자
    FCombatVisualizationData()
        : Attacker(nullptr)
        , Defender(nullptr)
        , CombatResult()
        , AttackerOriginalHexPosition(FVector2D::ZeroVector)
        , CombatRange(0)
        , bIsRangedAttack(false)
    {}

    // 전체 초기화 생성자
    FCombatVisualizationData(
        class AUnitCharacterBase* InAttacker,
        class AUnitCharacterBase* InDefender,
        const FCombatResult& InCombatResult,
        FVector2D InAttackerOriginalHexPosition,
        int32 InCombatRange,
        bool InIsRangedAttack
    )
        : Attacker(InAttacker)
        , Defender(InDefender)
        , CombatResult(InCombatResult)
        , AttackerOriginalHexPosition(InAttackerOriginalHexPosition)
        , CombatRange(InCombatRange)
        , bIsRangedAttack(InIsRangedAttack)
    {}

    // 결과 초기화 함수
    void Reset()
    {
        Attacker = nullptr;
        Defender = nullptr;
        CombatResult.Reset();
        AttackerOriginalHexPosition = FVector2D::ZeroVector;
        CombatRange = 0;
        bIsRangedAttack = false;
    }

    // 유효한 전투 시각화 데이터인지 확인
    bool IsValid() const
    {
        return Attacker != nullptr && 
               Defender != nullptr && 
               CombatResult.IsValid();
    }
};

