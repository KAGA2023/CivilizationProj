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

