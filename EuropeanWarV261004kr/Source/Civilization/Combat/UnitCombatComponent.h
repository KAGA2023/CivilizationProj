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
class UCityComponent;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class CIVILIZATION_API UUnitCombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:	
    UUnitCombatComponent();

    // ========== 전투 실행 ==========

    // 유닛 간 전투를 실행하고 결과를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombatResult ExecuteCombat(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, int32 HexDistance, FVector2D AttackerHex, FVector2D DefenderHex);

    // 전투가 가능한지 반환합니다. Hex를 주면 사거리·층수까지 검사합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanExecuteCombat(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, FVector2D AttackerHex = FVector2D::ZeroVector, FVector2D DefenderHex = FVector2D::ZeroVector) const;

    // 도시 공격이 가능한지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanExecuteCombatAgainstCity(AUnitCharacterBase* Attacker, UCityComponent* CityComponent, FVector2D AttackerHex = FVector2D::ZeroVector, FVector2D CityHex = FVector2D::ZeroVector) const;

    // 도시 공격을 실행하고 결과를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombatResult ExecuteCombatAgainstCity(AUnitCharacterBase* Attacker, UCityComponent* CityComponent, int32 HexDistance, FVector2D AttackerHex, FVector2D CityHex);

    // ========== 데미지 ==========

    // 현재 체력 비율을 반영한 공격 데미지를 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
    int32 CalculateAttackDamage(int32 BaseAttackStrength, int32 CurrentHealth, int32 MaxHealth) const;

    // 현재 체력 비율을 반영한 반격 데미지를 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
    int32 CalculateCounterDamage(int32 BaseDefenseStrength, int32 CurrentHealth, int32 MaxHealth) const;

    // 원거리 유닛의 지형 Range 보너스를 계산합니다.
    int32 CalculateRangeBonus(FVector2D MyHex) const;

    // ========== 지형 보너스 ==========

    // 층수와 숲을 합친 전투 보너스를 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    int32 CalculateCombatBonus(FVector2D MyHex, FVector2D EnemyHex) const;

    // 해당 hex의 지형 층수를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Combat")
    int32 GetFloorLevelAtHex(FVector2D HexPosition) const;

    // UI에 보여줄 지형 보너스 문구를 만듭니다.
    UFUNCTION(BlueprintCallable, Category = "Combat|UI")
    FText GetTerrainBonusText(FVector2D MyHex, FVector2D EnemyHex, int32 UnitRange) const;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // 체력 비율에 따라 실제 데미지를 줄입니다.
    int32 CalculateActualDamage(int32 BaseDamage, int32 CurrentHealth, int32 MaxHealth) const;

    // 상대보다 높은 층이면 보너스를 줍니다.
    int32 CalculateHeightBonus(FVector2D MyHex, FVector2D EnemyHex) const;

    // 숲 타일이면 보너스를 줍니다.
    int32 CalculateForestBonus(FVector2D MyHex) const;

    // 유닛의 스테이터스 컴포넌트를 반환합니다.
    UUnitStatusComponent* GetStatusComponent(AUnitCharacterBase* Unit) const;

public:	
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
