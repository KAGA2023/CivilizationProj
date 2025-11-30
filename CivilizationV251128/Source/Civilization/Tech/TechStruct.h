// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/Texture2D.h"
#include "TechStruct.generated.h"

// 기술 데이터 구조체 (데이터 테이블용)
USTRUCT(BlueprintType)
struct CIVILIZATION_API FTechData : public FTableRowBase
{
    GENERATED_BODY()

    // 기본 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Info")
    FString TechName;                                                          // 기술 이름

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Info")
    FString TechDescription;                                                      // 기술 설명

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tech Info")
    TSoftObjectPtr<UTexture2D> TechIcon;                                     // 기술 아이콘 이미지

    // 연구 비용
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research Cost")
    int32 ScienceCost = 0;                                                    // 연구에 필요한 과학 비용

    // 선행 기술
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Requirements")
    TArray<FName> PrerequisiteTechs;                                         // 선행 기술들 (RowName 배열)

    // 해제되는 시설
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlocks")
    TArray<FName> UnlockedFacilities;                                         // 해제되는 시설 목록 (RowName 배열)

    // 해제되는 건물
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlocks")
    TArray<FName> UnlockedBuildings;                                         // 해제되는 건물 목록 (RowName 배열)

    // 해제되는 유닛
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Unlocks")
    TArray<FName> UnlockedUnits;                                             // 해제되는 유닛 목록 (RowName 배열)

    FTechData()
    {
        TechName = TEXT("New Technology");
        TechDescription = TEXT("");
        ScienceCost = 0;
        PrerequisiteTechs.Empty();
        UnlockedFacilities.Empty();
        UnlockedBuildings.Empty();
        UnlockedUnits.Empty();
    }
};

// 기술 연구 상태 구조체 (게임 실행 중 변화하는 값들)
USTRUCT(BlueprintType)
struct CIVILIZATION_API FResearchCurrentStat
{
    GENERATED_BODY()

    // 연구 정보
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    FName DevelopingName = NAME_None;                                         // 개발중인 기술 이름

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    int32 DevelopingProgress = 0;                                             // 현재 개발 진행도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research")
    int32 DevelopingCost = 0;                                                 // 목표 개발 비용

    FResearchCurrentStat()
    {
        DevelopingName = NAME_None;
        DevelopingProgress = 0;
        DevelopingCost = 0;
    }
};

