// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UnitSpawnStruct.generated.h"

// 유닛 스폰 데이터 테이블용 구조체
USTRUCT(BlueprintType)
struct FUnitSpawnData : public FTableRowBase
{
    GENERATED_BODY()

    // 기본 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    FName UnitID = NAME_None;                    // 유닛 고유 ID

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unit Info")
    FName DataTableRowName = NAME_None;          // UnitDataStruct와 UnitStatusStruct 테이블의 공통 RowName

    // 팀 및 소속 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Team")
    int32 TeamID = 0;                           // 팀 ID (0: Player, 1: Enemy, 2: Neutral 등)

    // 스폰 관련
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FVector DefaultSpawnLocation = FVector::ZeroVector;    // 기본 스폰 위치

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    FRotator DefaultSpawnRotation = FRotator::ZeroRotator; // 기본 스폰 회전

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
    int32 MaxSpawnCount = 1;                     // 최대 스폰 가능 수

    // 생성자
    FUnitSpawnData()
    {
        UnitID = NAME_None;
        DataTableRowName = NAME_None;
        TeamID = 0;
        DefaultSpawnLocation = FVector::ZeroVector;
        DefaultSpawnRotation = FRotator::ZeroRotator;
        MaxSpawnCount = 1;
    }
};
