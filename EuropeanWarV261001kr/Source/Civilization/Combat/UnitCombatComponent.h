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

    // 전투 실행 (메인 함수)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombatResult ExecuteCombat(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, int32 HexDistance, FVector2D AttackerHex, FVector2D DefenderHex);

    // 전투 가능 여부 확인 (Hex 파라미터가 제공되면 사거리/층수 검증도 포함)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanExecuteCombat(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, FVector2D AttackerHex = FVector2D::ZeroVector, FVector2D DefenderHex = FVector2D::ZeroVector) const;

    // 도시 공격 가능 여부 확인
    UFUNCTION(BlueprintCallable, Category = "Combat")
    bool CanExecuteCombatAgainstCity(AUnitCharacterBase* Attacker, UCityComponent* CityComponent, FVector2D AttackerHex = FVector2D::ZeroVector, FVector2D CityHex = FVector2D::ZeroVector) const;

    // 도시 공격 실행
    UFUNCTION(BlueprintCallable, Category = "Combat")
    FCombatResult ExecuteCombatAgainstCity(AUnitCharacterBase* Attacker, UCityComponent* CityComponent, int32 HexDistance, FVector2D AttackerHex, FVector2D CityHex);

    // 공격 데미지 계산
    UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
    int32 CalculateAttackDamage(int32 BaseAttackStrength, int32 CurrentHealth, int32 MaxHealth) const;

    // 반격 데미지 계산
    UFUNCTION(BlueprintCallable, Category = "Combat|Damage")
    int32 CalculateCounterDamage(int32 BaseDefenseStrength, int32 CurrentHealth, int32 MaxHealth) const;

    // 원거리 유닛의 Range 보너스 계산 (지형 기반)
    int32 CalculateRangeBonus(FVector2D MyHex) const;

    // 지형 보너스 계산 (층수 + 숲)
    UFUNCTION(BlueprintCallable, Category = "Combat")
    int32 CalculateCombatBonus(FVector2D MyHex, FVector2D EnemyHex) const;

    // 타일의 층수 가져오기
    UFUNCTION(BlueprintCallable, Category = "Combat")
    int32 GetFloorLevelAtHex(FVector2D HexPosition) const;

    // 지형 보너스 텍스트 생성 (UI용)
    UFUNCTION(BlueprintCallable, Category = "Combat|UI")
    FText GetTerrainBonusText(FVector2D MyHex, FVector2D EnemyHex, int32 UnitRange) const;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // 체력 비율에 따른 데미지 배율 적용
    int32 CalculateActualDamage(int32 BaseDamage, int32 CurrentHealth, int32 MaxHealth) const;

    // 층수 보너스 계산
    int32 CalculateHeightBonus(FVector2D MyHex, FVector2D EnemyHex) const;

    // 숲 보너스 계산
    int32 CalculateForestBonus(FVector2D MyHex) const;

    // UnitStatusComponent 가져오기 헬퍼
    UUnitStatusComponent* GetStatusComponent(AUnitCharacterBase* Unit) const;

public:	
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

