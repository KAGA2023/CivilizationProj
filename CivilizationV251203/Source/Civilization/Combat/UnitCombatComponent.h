// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitCombatStruct.h"
#include "../Status/UnitStatusComponent.h"
#include "UnitCombatComponent.generated.h"

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

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // 체력 비율에 따른 데미지 배율 적용
    int32 CalculateActualDamage(int32 BaseDamage, int32 CurrentHealth, int32 MaxHealth) const;

    // 지형 보너스 가져오기 (HexPosition 기반)
    int32 GetCombatBonusAtHex(FVector2D HexPosition) const;

    // UnitStatusComponent 가져오기 헬퍼
    UUnitStatusComponent* GetStatusComponent(AUnitCharacterBase* Unit) const;

    // 지형 정보 가져오기 (향후 World/Grid 시스템과 연동)
    // ETerrainType GetTerrainTypeAtLocation(const FVector& Location) const;

public:	
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};

