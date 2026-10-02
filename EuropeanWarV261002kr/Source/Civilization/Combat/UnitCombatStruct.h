// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UnitCombatStruct.generated.h"

// 한 번의 전투에서 나온 생존·데미지 결과
USTRUCT(BlueprintType)
struct FCombatResult
{
    GENERATED_BODY()

    // ========== 생존 ==========

    // 공격자가 살아남았는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    bool bAttackerAlive = true;

    // 방어자가 살아남았는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    bool bDefenderAlive = true;

    // ========== 데미지 ==========

    // 공격자가 방어자에게 준 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    int32 AttackerDamageDealt = 0;

    // 방어자가 반격으로 준 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    int32 DefenderDamageDealt = 0;

    // 공격자가 반격으로 받은 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    int32 AttackerDamageTaken = 0;

    // 방어자가 공격으로 받은 데미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Result")
    int32 DefenderDamageTaken = 0;

    // 기본값으로 비어 있는 전투 결과를 만듭니다.
    FCombatResult()
        : bAttackerAlive(true)
        , bDefenderAlive(true)
        , AttackerDamageDealt(0)
        , DefenderDamageDealt(0)
        , AttackerDamageTaken(0)
        , DefenderDamageTaken(0)
    {}

    // 생존과 데미지를 한 번에 채웁니다.
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

    // 결과를 기본값으로 되돌립니다.
    void Reset()
    {
        bAttackerAlive = true;
        bDefenderAlive = true;
        AttackerDamageDealt = 0;
        DefenderDamageDealt = 0;
        AttackerDamageTaken = 0;
        DefenderDamageTaken = 0;
    }

    // 데미지 값이 음수가 아니면 유효한 결과로 봅니다.
    bool IsValid() const
    {
        return AttackerDamageDealt >= 0 && 
               DefenderDamageDealt >= 0 && 
               AttackerDamageTaken >= 0 && 
               DefenderDamageTaken >= 0;
    }
};
