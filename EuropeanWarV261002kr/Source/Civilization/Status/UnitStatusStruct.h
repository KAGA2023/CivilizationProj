// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "../World/WorldStruct.h"
#include "UnitStatusStruct.generated.h"

// 유닛 병과
UENUM(BlueprintType)
enum class EUnitClass : uint8
{
    None                UMETA(DisplayName = "None"),
    Worker              UMETA(DisplayName = "Worker"),
    Sword               UMETA(DisplayName = "Sword"),
    Bow                 UMETA(DisplayName = "Bow"),
    ShortSword          UMETA(DisplayName = "ShortSword"),
    Shield              UMETA(DisplayName = "Shield"),
    Hammer              UMETA(DisplayName = "Hammer"),
    GreatSword          UMETA(DisplayName = "GreatSword"),
    CrossBow            UMETA(DisplayName = "CrossBow"),
};

// 유닛 스탯에 더하는 보정값
USTRUCT(BlueprintType)
struct FUnitStatModifier
{
    GENERATED_BODY()

    // 체력 보정
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddHealth = 0;

    // 공격 시 공격력 보정
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddAttackStrength = 0;

    // 반격 시 공격력 보정
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddDefenseStrength = 0;

    // 이동력 보정
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddMovementPoints = 0;

    // 사거리 보정
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AddRange = 0;

    bool operator==(const FUnitStatModifier& Other) const;
    FUnitStatModifier operator+(const FUnitStatModifier& Other) const;
    FUnitStatModifier operator-(const FUnitStatModifier& Other) const;
    void operator+=(const FUnitStatModifier& Other);
    void operator-=(const FUnitStatModifier& Other);
    void Reset();
};

// 유닛 스테이터스 데이터 테이블 한 줄
USTRUCT(BlueprintType)
struct FUnitBaseStat : public FTableRowBase
{
    GENERATED_BODY()

    // ========== 기본 정보 ==========

    // 유닛 병과
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    EUnitClass UnitClass = EUnitClass::None;

    // 유닛 표시 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    FText UnitName;

    // 유닛 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    FText UnitDescription;

    // 유닛 아이콘 이미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    TSoftObjectPtr<UTexture2D> UnitIcon;

    // ========== 전투 ==========

    // 최대 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    int32 MaxHealth = 0;

    // 공격 시 공격력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    int32 AttackStrength = 0;

    // 반격 시 공격력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat Stats")
    int32 DefenseStrength = 0;

    // ========== 이동 / 사거리 ==========

    // 한 턴 이동력
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    int32 MovementPoints = 2;

    // 공격 사거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
    int32 Range = 0;

    // ========== 생산 비용 ==========

    // 생산에 필요한 식량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production & Cost")
    int32 FoodCost = 0;

    // 골드로 즉시 살 때 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production & Cost")
    int32 GoldCost = 0;

    // 필요한 전략 자원 종류
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production & Cost")
    TArray<EStrategicResource> RequiredResources;

    // 필요한 전략 자원 수량
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Production & Cost")
    TArray<int32> RequiredResourceAmounts;

    // ========== 특수 능력 ==========

    // 공격과 도시 점령이 가능한지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Abilities")
    bool CanAttack = false;

    // 시설을 지을 수 있는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Special Abilities")
    bool CanBuildFacilities = false;
};

// 게임 중 바뀌는 유닛 체력·이동력·턴 상태
USTRUCT(BlueprintType)
struct FUnitCurrentStat
{
    GENERATED_BODY()

    // 남은 체력
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RemainingHealth = 0;

    // 이번 턴 남은 이동력
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 RemainingMovementPoints = 0;

    // 이번 턴에 이미 공격했는지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool HasAttacked = false;

    // 대기 상태인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsWait = false;

    // 경계 상태인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsAlert = false;

    // 휴면 상태인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool IsSleep = false;
};

// 모디파이어를 반영한 최종 유닛 스탯
USTRUCT(BlueprintType)
struct FUnitFinalStat
{
    GENERATED_BODY()

    // 최종 공격력
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 AttackStrength = 0;

    // 최종 반격 공격력
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 DefenseStrength = 0;

    // 최종 이동력
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MovementPoints = 0;

    // 최종 사거리
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Range = 0;

    // 최종 식량 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FoodCost = 0;

    // 최종 골드 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 GoldCost = 0;
};
