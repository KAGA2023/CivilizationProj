// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Engine/StaticMesh.h"
#include "../World/WorldStruct.h"
#include "FacilityStruct.generated.h"

// 시설 타입 열거형 (카테고리)
UENUM(BlueprintType)
enum class EFacilityType : uint8
{
    None                    UMETA(DisplayName = "None"),                    // 없음
    General                 UMETA(DisplayName = "General"),                 // 일반시설
    Agricultural            UMETA(DisplayName = "Agricultural"),            // 식량시설
    Industrial              UMETA(DisplayName = "Industrial"),              // 산업시설
    Commercial              UMETA(DisplayName = "Commercial"),              // 상업시설
    Religious               UMETA(DisplayName = "Religious"),               // 종교시설
    Scientific              UMETA(DisplayName = "Scientific"),              // 과학시설
    Military                UMETA(DisplayName = "Military"),               // 군사시설
    Resource                UMETA(DisplayName = "Resource")                 // 자원시설
};

// 시설 데이터 구조체 (데이터 테이블용)
USTRUCT(BlueprintType)
struct CIVILIZATION_API FFacilityData : public FTableRowBase
{
    GENERATED_BODY()

    // 기본 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility Info")
    EFacilityType FacilityType = EFacilityType::None;                     // 시설 타입

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility Info")
    FString FacilityName;                                                   // 시설 이름

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Facility Info")
    FString Description;                                                    // 시설 설명

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
    bool bRequiresForest = false;                                           // 숲이 필요한지 여부

    // 시각적 표현
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* FacilityMesh = nullptr;                                    // 시설 메시

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* PillagedMesh = nullptr;                                    // 약탈 상태 메시

    // 필요 기술
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    FName RequiredTechnology = NAME_None;                                   // 필요 기술

    FFacilityData()
    {
        FacilityType = EFacilityType::None;
        FacilityName = TEXT("New Facility");
        Description = TEXT("");
        RequiredTechnology = NAME_None;
        bIsPillaged = false;
        CompatibleLandTypes.Empty();
        CompatibleClimates.Empty();
        CompatibleBonusResources.Empty();
        CompatibleLuxuryResources.Empty();
        CompatibleStrategicResources.Empty();
        bRequiresForest = false;
        FacilityMesh = nullptr;
        PillagedMesh = nullptr;
    }
};

