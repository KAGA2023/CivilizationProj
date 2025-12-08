// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitCombatStruct.h"
#include "../Status/UnitStatusComponent.h"
#include "UnitCombatComponent.generated.h"

// 지형 보너스 수치 정의
#define LAND_ATK_BONUS 5

class UUnitStatusComponent;
class AUnitCharacterBase;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CIVILIZATION_API UUnitCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:	
    UUnitCombatComponent();

    // 전투 실행 (메인 함수)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombatResult ExecuteCombat(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, int32 HexDistance, FVector2D AttackerHex, FVector2D DefenderHex);

    // 전투 가능 여부 확인
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanExecuteCombat(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender) const;

    // 공격 데미지 계산
    UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
    int32 CalculateAttackDamage(int32 BaseAttackStrength, int32 CurrentHealth, int32 MaxHealth) const;

    // 반격 데미지 계산
    UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
    int32 CalculateCounterDamage(int32 BaseDefenseStrength, int32 CurrentHealth, int32 MaxHealth) const;

     // 원거리 유닛의 Range 보너스 계산 (지형 기반)
     int32 CalculateRangeBonus(FVector2D MyHex) const;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // 체력 비율에 따른 데미지 배율 적용
    int32 CalculateActualDamage(int32 BaseDamage, int32 CurrentHealth, int32 MaxHealth) const;

    // 지형 보너스 계산 (층수 + 숲)
    int32 CalculateCombatBonus(FVector2D MyHex, FVector2D EnemyHex) const;

    // 층수 보너스 계산
    int32 CalculateHeightBonus(FVector2D MyHex, FVector2D EnemyHex) const;

    // 숲 보너스 계산
    int32 CalculateForestBonus(FVector2D MyHex) const;

    // 타일의 층수 가져오기
    int32 GetFloorLevelAtHex(FVector2D HexPosition) const;

    // UnitStatusComponent 가져오기 헬퍼
    UUnitStatusComponent* GetStatusComponent(AUnitCharacterBase* Unit) const;

public:	
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

