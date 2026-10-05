// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "Research.generated.h"

// ========== 기술 데이터 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FTechData : public FTableRowBase
{
    GENERATED_BODY()

    // 기술 이름
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Info")
    FString TechName;

    // 기술 설명
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Info")
    FString TechDescription;

    // 기술 아이콘 이미지
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Info")
    TSoftObjectPtr<UTexture2D> TechIcon;

    // 연구에 필요한 과학 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research Cost")
    int32 ScienceCost = 0;

    // 선행 기술 RowName 배열
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    TArray<FName> RequiredTechs;

    // 이 기술이 해제하는 시설 RowName 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlocks")
    TArray<FName> UnlockFacilities;

    // 이 기술이 해제하는 건물 RowName 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlocks")
    TArray<FName> UnlockBuildings;

    // 이 기술이 해제하는 유닛 RowName 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlocks")
    TArray<FName> UnlockUnits;

    FTechData()
    {
        TechName = TEXT("New Technology");
        TechDescription = TEXT("");
        ScienceCost = 0;
        RequiredTechs.Empty();
        UnlockFacilities.Empty();
        UnlockBuildings.Empty();
        UnlockUnits.Empty();
    }
};

// ========== 연구 완료 목록 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FResearchData
{
    GENERATED_BODY()

    // 연구 완료된 기술 RowName 목록
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech System")
    TArray<FName> ResearchedTechs;

    FResearchData()
    {
        ResearchedTechs.Empty();
    }
};

// ========== 현재 연구 상태 ==========

USTRUCT(BlueprintType)
struct CIVILIZATION_API FResearchCurrentStat
{
    GENERATED_BODY()

    // 지금 개발 중인 기술 RowName
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    FName DevelopingName = NAME_None;

    // 현재 개발 진행도
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    int32 DevelopingProgress = 0;

    // 목표 개발 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    int32 DevelopingCost = 0;

    FResearchCurrentStat()
    {
        DevelopingName = NAME_None;
        DevelopingProgress = 0;
        DevelopingCost = 0;
    }
};
