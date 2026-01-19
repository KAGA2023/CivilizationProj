// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "../World/WorldStruct.h"
#include "FacilityStruct.generated.h"

// 시설 데이터 구조체 (데이터 테이블용)
USTRUCT(BlueprintType)
struct CIVILIZATION_API FFacilityData : public FTableRowBase
{
    GENERATED_BODY()

    // 기본 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility Info")
    FString FacilityName;                                                   // 시설 이름

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility Info")
    FString FacilityDescription;                                            // 시설 설명

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility Info")
    TSoftObjectPtr<UTexture2D> FacilityIcon;                               // 시설 아이콘 이미지

    // 타일 모디파이어 (타일에 건설 시 생산량 변화)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    FTileModifier TileModifier;                                             // 타일 생산량 모디파이어

    // 시설 상태
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility State")
    bool bIsPillaged = false;                                               // 약탈 여부

    // 건설 가능 조건
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    TArray<ELandType> CompatibleLandTypes;                                  // 호환되는 지형 타입들

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    TArray<EClimateType> CompatibleClimates;                                // 호환되는 기후대들

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    TArray<EBonusResource> CompatibleBonusResources;                         // 호환되는 보너스 자원들

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    TArray<ELuxuryResource> CompatibleLuxuryResources;                         // 호환되는 사치 자원들

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    TArray<EStrategicResource> CompatibleStrategicResources;                   // 호환되는 전략 자원들

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    bool bIsForest = false;                                                 // true: 숲이 있는 타일에만 건설, false: 숲이 없는 타일에만 건설

    // 시각적 표현
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* FacilityMesh = nullptr;                                    // 시설 메시

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* PillagedMesh = nullptr;                                    // 약탈 상태 메시

    FFacilityData()
    {
        FacilityName = TEXT("New Facility");
        FacilityDescription = TEXT("");
        bIsPillaged = false;
        bIsForest = false;
        FacilityMesh = nullptr;
        PillagedMesh = nullptr;
    }
};

