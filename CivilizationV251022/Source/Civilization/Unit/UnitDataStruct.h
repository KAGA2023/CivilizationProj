// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UnitDataStruct.generated.h"

class USkeletalMesh;
class UAnimInstance;
class USoundBase;
class UUserWidget;

// 유닛 데이터 테이블용 구조체
USTRUCT(BlueprintType)
struct FUnitData : public FTableRowBase
{
    GENERATED_BODY()

    // 시각적 요소들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    USkeletalMesh* UnitMesh = nullptr;

    // 애니메이션
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSubclassOf<class UAnimInstance> AnimClass;

    // 사운드 효과
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* MoveSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* AttackSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* DeathSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* SelectSound = nullptr;

    // UI 관련
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    TSubclassOf<class UUserWidget> HealthBarWidgetClass;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
    FVector HealthBarOffset = FVector(0.f, 0.f, 200.f);
};

