// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UnitStatusStruct.h"
#include "GameFramework/Actor.h"
#include "UnitStatusComponent.generated.h"

class UDataTable;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CIVILIZATION_API UUnitStatusComponent : public UActorComponent
{
    GENERATED_BODY()

protected:
    // ========== 스탯 데이터 ==========

    // 데이터 테이블에서 온 기본 스탯
    UPROPERTY(BlueprintReadOnly, Category = "Unit Status")
    FUnitBaseStat m_BaseStat;

    // 체력·이동력·턴 상태
    UPROPERTY(BlueprintReadOnly, Category = "Unit Status")
    FUnitCurrentStat m_CurrentStat;

    // 적용 중인 스탯 모디파이어
    UPROPERTY()
    TArray<FUnitStatModifier> m_StatModifiers;

    // 모디파이어를 반영한 최종 스탯
    UPROPERTY(BlueprintReadOnly, Category = "Unit Status")
    FUnitFinalStat m_FinalStat;

public:	
    UUnitStatusComponent();

    // ========== 초기화 ==========

    // 준비된 기본 스탯으로 현재·최종 값을 맞춥니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void InitFromBaseStat(const FUnitBaseStat& InBaseStat);

    // RowName으로 데이터 테이블에서 기본 스탯을 읽습니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void InitializeFromRowName(FName RowName);

    // ========== 스탯 접근 ==========

    // 기본 스탯을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    FUnitBaseStat GetBaseStat() const { return m_BaseStat; }

    // 현재 체력·이동력·턴 상태를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    FUnitCurrentStat GetCurrentStat() const { return m_CurrentStat; }

    // 최종 스탯을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    FUnitFinalStat GetFinalStat() const { return m_FinalStat; }

    // 최종 공격력을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetAttackStrength() const { return m_FinalStat.AttackStrength; }

    // 최종 반격 공격력을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetDefenseStrength() const { return m_FinalStat.DefenseStrength; }

    // 최종 이동력을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetMovementPoints() const { return m_FinalStat.MovementPoints; }

    // 최종 사거리를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetRange() const { return m_FinalStat.Range; }

    // 최종 골드 비용을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetGoldCost() const { return m_FinalStat.GoldCost; }

    // 최종 식량 비용을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetFoodCost() const { return m_FinalStat.FoodCost; }

    // ========== 모디파이어 ==========

    // 스탯 모디파이어를 넣고 최종 값을 다시 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void AddStatModifier(const FUnitStatModifier& Modifier);

    // 스탯 모디파이어를 빼고 최종 값을 다시 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void RemoveStatModifier(const FUnitStatModifier& Modifier);

    // 모든 모디파이어를 지웁니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void ClearAllModifiers();

    // 기본 스탯과 모디파이어로 최종 값을 다시 계산합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void RecalculateStats();

    // ========== 턴 / 이동 ==========

    // 새 턴에 맞춰 이동력과 공격 여부를 되돌립니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void ResetTurn();

    // 남은 이동력을 깎습니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void ConsumeMovement(int32 Amount);

    // 지금 이동할 수 있는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    bool CanMove() const;

    // 지금 공격할 수 있는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    bool CanAttack() const;

    // ========== 상태 설정 ==========

    // 이번 턴 공격 여부를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void SetHasAttacked(bool bHasAttacked);

    // 대기 상태를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void SetWait(bool bWait);

    // 경계 상태를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void SetAlert(bool bAlert);

    // 휴면 상태를 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void SetSleep(bool bSleep);

    // ========== 체력 ==========

    // 체력을 깎습니다. 0이면 사망 처리합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void TakeDamage(int32 DamageAmount);

    // 체력을 회복합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void Heal(int32 HealAmount);

    // 남은 체력을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetCurrentHealth() const { return m_CurrentStat.RemainingHealth; }

    // 최대 체력을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetMaxHealth() const;

    // 체력이 0 이하인지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    bool IsDead() const;

    // 기본 스탯이 유효한지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    bool IsValid() const;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // 유닛 스테이터스 데이터 테이블
    UPROPERTY()
    UDataTable* UnitStatusTable = nullptr;

    // 스테이터스 테이블을 로드합니다.
    void LoadUnitStatusTable();

    // 모든 모디파이어를 합친 값을 반환합니다.
    FUnitStatModifier CalculateTotalModifier() const;

    // 체력이 0이 되면 유닛을 제거합니다.
    void OnUnitDeath();

public:	
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
