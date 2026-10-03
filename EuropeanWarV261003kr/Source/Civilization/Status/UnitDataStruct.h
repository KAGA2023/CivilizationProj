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

// 무기를 붙일 스켈레톤 소켓
UENUM(BlueprintType)
enum class EUnitWeaponSocket : uint8
{
	ShieldSocket,    // 방패
	BowSocket,       // 활
	CrossBowSocket,  // 석궁
	TwoHandSocket,   // 양손 무기
	OneHandSocket    // 한손 무기
};

// 유닛 외형·애님·사운드 데이터 테이블 한 줄
USTRUCT(BlueprintType)
struct FUnitData : public FTableRowBase
{
    GENERATED_BODY()

    // ========== 외형 ==========

    // 유닛 스켈레탈 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    USkeletalMesh* UnitMesh = nullptr;

    // 손에 붙일 무기 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    UStaticMesh* WeaponMesh = nullptr;

    // 무기를 붙일 스켈레톤 소켓
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
    EUnitWeaponSocket WeaponSocket = EUnitWeaponSocket::OneHandSocket;

    // ========== 애니메이션 ==========

    // 사용할 애님 인스턴스 클래스
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    TSubclassOf<class UAnimInstance> AnimClass;

    // 공격 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* AttackMontage = nullptr;

    // 사망 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* DeathMontage = nullptr;

    // 피격 몽타주
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
    UAnimMontage* HitMontage = nullptr;

    // ========== 사운드 ==========

    // 피격 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* HitSound = nullptr;

    // 공격 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* AttackSound = nullptr;

    // 사망 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* DeathSound = nullptr;

    // 선택 사운드
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundBase* SelectSound = nullptr;
};
