// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "../World/WorldStruct.h"
#include "FacilityStruct.generated.h"

// 시설 데이터 테이블 한 줄. 이름·생산 보정·건설 조건·메시를 담습니다.
USTRUCT(BlueprintType)
struct CIVILIZATION_API FFacilityData : public FTableRowBase
{
    GENERATED_BODY()

    // ========== 기본 정보 ==========

    // 시설 표시 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility Info")
    FString FacilityName;

    // 시설 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility Info")
    FString FacilityDescription;

    // 시설 아이콘 이미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility Info")
    TSoftObjectPtr<UTexture2D> FacilityIcon;

    // ========== 타일 보정 ==========

    // 건설 시 타일 생산량에 더하는 모디파이어
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tile Modifier")
    FTileModifier TileModifier;

    // ========== 상태 ==========

    // 지금 약탈된 상태인지 여부
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility State")
    bool bIsPillaged = false;

    // ========== 건설 조건 ==========

    // 건설 가능한 지형 타입
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    TArray<ELandType> CompatibleLandTypes;

    // 건설 가능한 기후대
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    TArray<EClimateType> CompatibleClimates;

    // 필요한 보너스 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    TArray<EBonusResource> CompatibleBonusResources;

    // 필요한 사치 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    TArray<ELuxuryResource> CompatibleLuxuryResources;

    // 필요한 전략 자원
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    TArray<EStrategicResource> CompatibleStrategicResources;

    // true면 숲 타일에만, false면 숲 없는 타일에만 건설합니다.
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Requirements")
    bool bIsForest = false;

    // ========== 외형 ==========

    // 정상 상태 시설 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* FacilityMesh = nullptr;

    // 약탈 상태 시설 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* PillagedMesh = nullptr;

    // 숲만 있는 타일에 건설할 때 쓰는 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual | Forest")
    UStaticMesh* ForestFacilityMesh = nullptr;

    // 숲만 있는 타일에서 약탈됐을 때 쓰는 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual | Forest")
    UStaticMesh* ForestFacilityPillagedMesh = nullptr;

    FFacilityData()
    {
        FacilityName = TEXT("New Facility");
        FacilityDescription = TEXT("");
        bIsPillaged = false;
        bIsForest = false;
        FacilityMesh = nullptr;
        PillagedMesh = nullptr;
        ForestFacilityMesh = nullptr;
        ForestFacilityPillagedMesh = nullptr;
    }
};
