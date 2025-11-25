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
    // 기본 스테이터스 데이터
    UPROPERTY(BlueprintReadOnly, Category = "Unit Status")
    FUnitBaseStat m_BaseStat;

    // 현재 스테이터스 데이터
    UPROPERTY(BlueprintReadOnly, Category = "Unit Status")
    FUnitCurrentStat m_CurrentStat;

    // 스테이터스 모디파이어 배열
    UPROPERTY()
    TArray<FUnitStatModifier> m_StatModifiers;

    // 계산된 최종 스테이터스
    UPROPERTY(BlueprintReadOnly, Category = "Unit Status")
    FUnitFinalStat m_FinalStat;

public:	
    UUnitStatusComponent();

    // 초기화 함수 (외부에서 준비된 BaseStat을 받아서 초기화)
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void InitFromBaseStat(const FUnitBaseStat& InBaseStat);

    // 초기화 함수 (RowName으로 데이터 테이블에서 직접 로드)
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void InitializeFromRowName(FName RowName);

    // 스테이터스 접근 함수들
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    FUnitBaseStat GetBaseStat() const { return m_BaseStat; }

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    FUnitCurrentStat GetCurrentStat() const { return m_CurrentStat; }

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    FUnitFinalStat GetFinalStat() const { return m_FinalStat; }

    // 최종 스테이터스 접근 (계산된 값)
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetAttackStrength() const { return m_FinalStat.AttackStrength; }

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetDefenseStrength() const { return m_FinalStat.DefenseStrength; }

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetMovementPoints() const { return m_FinalStat.MovementPoints; }

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetSightRange() const { return m_FinalStat.SightRange; }

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetRange() const { return m_FinalStat.Range; }

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetGoldCost() const { return m_FinalStat.GoldCost; }

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetFoodCost() const { return m_FinalStat.FoodCost; }

    // 모디파이어 관리
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void AddStatModifier(const FUnitStatModifier& Modifier);

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void RemoveStatModifier(const FUnitStatModifier& Modifier);

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void ClearAllModifiers();

    // 스테이터스 재계산
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void RecalculateStats();

    // 현재 상태 관리
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void ResetTurn();

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void ConsumeMovement(int32 Amount);

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    bool CanMove() const;

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    bool CanAttack() const;

    // 상태 설정
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void SetWait(bool bWait);

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void SetAlert(bool bAlert);

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void SetSleep(bool bSleep);

    // 체력 관리
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void TakeDamage(int32 DamageAmount);

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    void Heal(int32 HealAmount);

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetCurrentHealth() const { return m_CurrentStat.RemainingHealth; }

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    int32 GetMaxHealth() const;

    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    bool IsDead() const;

    // 유틸리티 함수들
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    bool IsValid() const;

protected:
    // Called when the game starts
    virtual void BeginPlay() override;

    // 유닛 스테이터스 데이터 테이블
    UPROPERTY()
    UDataTable* UnitStatusTable = nullptr;

    // 내부 계산 함수들
    void LoadUnitStatusTable();
    FUnitStatModifier CalculateTotalModifier() const;
    void OnUnitDeath();

public:	
    // Called every frame
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
