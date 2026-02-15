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

// 유닛에 무기 부착 시 사용할 소켓 (5가지 중 선택)
UENUM(BlueprintType)
enum class EUnitWeaponSocket : uint8
{
	ShieldSocket,
	BowSocket,
	CrossBowSocket,
	TwoHandSocket,
	OneHandSocket
};

// 유닛 데이터 테이블용 구조체
USTRUCT(BlueprintType)
struct FUnitData : public FTableRowBase
{
    GENERATED_BODY()

    // 시각적 요소들
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    USkeletalMesh* UnitMesh = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* WeaponMesh = nullptr;

    /** 유닛 스켈레톤에 무기(WeaponMesh)를 부착할 소켓. 5가지 중 선택 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    EUnitWeaponSocket WeaponSocket = EUnitWeaponSocket::OneHandSocket;

    // 애니메이션
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSubclassOf<class UAnimInstance> AnimClass;

    // 애니메이션 Montage
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* AttackMontage = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* DeathMontage = nullptr;

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

