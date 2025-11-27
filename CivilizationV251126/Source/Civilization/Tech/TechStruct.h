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
    FString Description;                                                      // 기술 설명

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
        Description = TEXT("");
        ScienceCost = 0;
        PrerequisiteTechs.Empty();
        UnlockedFacilities.Empty();
        UnlockedBuildings.Empty();
        UnlockedUnits.Empty();
    }
};

// 기술 연구 상태 구조체 (게임 실행 중 변화하는 값들)
USTRUCT(BlueprintType)
struct CIVILIZATION_API FTechResearchStatus
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research Status")
    FName TechRowName = NAME_None;                                           // 기술 RowName

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research Status")
    int32 CurrentProgress = 0;                                                // 현재 연구 진행도

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research Status")
    bool bIsResearched = false;                                               // 연구 완료 여부

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Research Status")
    bool bIsResearching = false;                                              // 현재 연구 중인지 여부

    FTechResearchStatus()
    {
        TechRowName = NAME_None;
        CurrentProgress = 0;
        bIsResearched = false;
        bIsResearching = false;
    }

    FTechResearchStatus(FName InTechRowName)
    {
        TechRowName = InTechRowName;
        CurrentProgress = 0;
        bIsResearched = false;
        bIsResearching = false;
    }
};

