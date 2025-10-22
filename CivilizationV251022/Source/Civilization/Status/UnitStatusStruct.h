// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "../WorldStruct.h"
#include "UnitStatusStruct.generated.h"

// 유닛 클래스 열거형
UENUM(BlueprintType)
enum class EUnitClass : uint8
{
    None                UMETA(DisplayName = "None"),
    // 전투 유닛
    Warrior             UMETA(DisplayName = "Warrior"),           // 전사
    Archer              UMETA(DisplayName = "Archer"),            // 궁수
    Spearman            UMETA(DisplayName = "Spearman"),          // 창병
    Horseman            UMETA(DisplayName = "Horseman"),          // 기병
    Knight              UMETA(DisplayName = "Knight"),            // 기사
    // 특수 유닛
    Builder             UMETA(DisplayName = "Builder"),          // 건설자
    Scout               UMETA(DisplayName = "Scout"),             // 정찰병
};

// 스테이터스 수정자 구조체
USTRUCT(BlueprintType)
struct FUnitStatModifier
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddHealth = 0;                     // 체력

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddAttackStrength = 0;             // 공격시 공격력

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddDefenseStrength = 0;            // 반격시 공격력

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddMovementPoints = 0;             // 이동력

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddSightRange = 0;                 // 시야 범위

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddRange = 0;                      // 사거리

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddProductionCost = 0;              // 생산 비용 (음수로 할인)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddGoldCost = 0;                   // 골드 비용 (음수로 할인)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddFaithCost = 0;                  // 신앙 비용 (음수로 할인)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddMaintenanceCost = 0;            // 턴당 골드 유지비 (음수로 할인)

    // 연산자 오버로드
    bool operator==(const FUnitStatModifier& Other) const;
    FUnitStatModifier operator+(const FUnitStatModifier& Other) const;
    FUnitStatModifier operator-(const FUnitStatModifier& Other) const;
    void operator+=(const FUnitStatModifier& Other);
    void operator-=(const FUnitStatModifier& Other);
    void Reset();
};

// 기본 스테이터스 구조체 (데이터 테이블용)
USTRUCT(BlueprintType)
struct FUnitBaseStat : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EUnitClass UnitClass = EUnitClass::None;     // 유닛 클래스

    // 기본 전투 스테이터스
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaxHealth = 0;                      // 최대 체력

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AttackStrength = 0;                 // 공격시 공격력

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DefenseStrength = 0;                // 반격시 공격력

    // 이동 및 시야
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MovementPoints = 2;                 // 이동력

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SightRange = 2;                     // 시야 범위

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Range = 0;                          // 사거리

    // 생산 및 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ProductionCost = 0;                   // 생산 비용

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoldCost = 0;                         // 골드 비용

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FaithCost = 0;                        // 신앙 비용

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaintenanceCost = 0;                  // 턴당 골드 유지비

    // 특수 능력
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool CanAttack = false;                    // 공격 가능 (도시 점령도 포함)

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool CanBuildBuildings = false;            // 건물 건설 가능

    // 기술 요구사항
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName RequiredTechnology = NAME_None;      // 필요 기술

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<EStrategicResource> RequiredResources; // 필요 전략 자원들

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<int32> RequiredResourceAmounts;        // 필요 전략 자원 수량들

    // 설명 텍스트
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText UnitName;                            // 유닛 이름

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FText UnitDescription;                     // 유닛 설명
};

// 현재 스테이터스 구조체
USTRUCT(BlueprintType)
struct FUnitCurrentStat
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RemainingMovementPoints = 0;         // 남은 이동력

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool HasAttacked = false;                  // 이번 턴에 공격했는지

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsWait = false;                       // 대기 상태

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsAlert = false;                      // 경계 상태

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsSleep = false;                      // 휴면 상태

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RemainingHealth = 0;                 // 남은 체력
};

// 최종 스테이터스 구조체 (계산된 값들)
USTRUCT(BlueprintType)
struct FUnitFinalStat
{
    GENERATED_BODY()

    // 기본 전투 스테이터스
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AttackStrength = 0;             // 최종 공격시 공격력

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DefenseStrength = 0;            // 최종 반격시 공격력

    // 이동 및 시야
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MovementPoints = 0;             // 최종 이동력

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 SightRange = 0;                 // 최종 시야 범위

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Range = 0;                      // 최종 사거리

    // 생산 및 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 ProductionCost = 0;             // 최종 생산 비용

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoldCost = 0;                   // 최종 골드 비용

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FaithCost = 0;                  // 최종 신앙 비용

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MaintenanceCost = 0;            // 최종 턴당 골드 유지비
};
