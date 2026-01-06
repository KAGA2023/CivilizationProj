// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "UnitDataStruct.generated.h"

class USkeletalMesh;
class UStaticMesh;
class UAnimInstance;
class UAnimMontage;
class USoundBase;

// 유닛 데이터 테이블용 구조체
USTRUCT(BlueprintType)
struct FUnitData : public FTableRowBase
{
    GENERATED_BODY()

    // 시각적 요소들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    USkeletalMesh* UnitMesh = nullptr;

    // 무기 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* WeaponMesh = nullptr;

    // 무기 소켓 이름 (스켈레탈 메시의 소켓에 부착)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    FName WeaponSocketName = NAME_None;

    // 애니메이션
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSubclassOf<class UAnimInstance> AnimClass;

    // 애니메이션 Montage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* AttackMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* DeathMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* SelectMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* HitMontage = nullptr;

    // 사운드 효과
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* MoveSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* AttackSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* DeathSound = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* SelectSound = nullptr;
};

