// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitCombatComponent.h"
#include "../Status/UnitStatusComponent.h"
#include "../Status/UnitStatusStruct.h"
#include "../Unit/UnitCharacterBase.h"

UUnitCombatComponent::UUnitCombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UUnitCombatComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UUnitCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

FCombatResult UUnitCombatComponent::ExecuteCombat(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender)
{
    FCombatResult Result;

    // 1. 전투 전 검증
    if (!CanExecuteCombat(Attacker, Defender))
    {
        return Result;
    }

    // 2. 컴포넌트 가져오기
    UUnitStatusComponent* AttackerStatus = GetStatusComponent(Attacker);
    UUnitStatusComponent* DefenderStatus = GetStatusComponent(Defender);

    if (!AttackerStatus || !DefenderStatus)
    {
        return Result;
    }

    // 3. 스탯 가져오기
    int32 AttackerAttackStrength = AttackerStatus->GetAttackStrength();
    int32 DefenderDefenseStrength = DefenderStatus->GetDefenseStrength();
    int32 AttackerCurrentHealth = AttackerStatus->GetCurrentHealth();
    int32 AttackerMaxHealth = AttackerStatus->GetMaxHealth();
    int32 DefenderCurrentHealth = DefenderStatus->GetCurrentHealth();
    int32 DefenderMaxHealth = DefenderStatus->GetMaxHealth();

    // 4. 지형 정보 가져오기 (향후 확장용, 현재는 기본값 사용)
    FVector AttackerLocation = Attacker->GetActorLocation();
    FVector DefenderLocation = Defender->GetActorLocation();

    // 5. 데미지 계산 (동시에 계산)
    int32 AttackDamage = CalculateAttackDamage(AttackerAttackStrength, AttackerCurrentHealth, AttackerMaxHealth);
    int32 CounterDamage = CalculateCounterDamage(DefenderDefenseStrength, DefenderCurrentHealth, DefenderMaxHealth);

    // 6. 데미지 적용 (동시에 적용 - 문명6 방식)
    // 피격자가 먼저 죽어도 반격 데미지는 공격자에게 들어감
    DefenderStatus->TakeDamage(AttackDamage);
    AttackerStatus->TakeDamage(CounterDamage);

    // 7. 결과 확인
    Result.bDefenderAlive = !DefenderStatus->IsDead();
    Result.bAttackerAlive = !AttackerStatus->IsDead();
    Result.AttackerDamageDealt = AttackDamage;
    Result.DefenderDamageDealt = CounterDamage;
    Result.AttackerDamageTaken = CounterDamage;
    Result.DefenderDamageTaken = AttackDamage;

    // 8. 상태 업데이트
    AttackerStatus->SetHasAttacked(true);

    return Result;
}

bool UUnitCombatComponent::CanExecuteCombat(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender) const
{
    // 1. 유효성 검사
    if (!Attacker || !Defender)
    {
        return false;
    }

    if (Attacker == Defender)
    {
        return false; // 자기 자신을 공격할 수 없음
    }

    // 2. 컴포넌트 검사
    UUnitStatusComponent* AttackerStatus = GetStatusComponent(Attacker);
    UUnitStatusComponent* DefenderStatus = GetStatusComponent(Defender);

    if (!AttackerStatus || !DefenderStatus)
    {
        return false;
    }

    // 3. 유효한 유닛인지 확인
    if (!AttackerStatus->IsValid() || !DefenderStatus->IsValid())
    {
        return false;
    }

    // 4. 이미 죽은 유닛인지 확인
    if (AttackerStatus->IsDead() || DefenderStatus->IsDead())
    {
        return false;
    }

    // 5. 공격 가능 여부 확인
    if (!AttackerStatus->CanAttack())
    {
        return false;
    }

    // 6. 사거리 확인 (향후 확장)
    // int32 AttackRange = AttackerStatus->GetRange();
    // float Distance = FVector::Dist(Attacker->GetActorLocation(), Defender->GetActorLocation());
    // if (Distance > AttackRange) return false;

    return true;
}

int32 UUnitCombatComponent::CalculateAttackDamage(int32 BaseAttackStrength, int32 CurrentHealth, int32 MaxHealth) const
{
    // 지형 보너스는 향후 확장 (현재는 0.0f)
    float TerrainBonus = 0.0f; // GetTerrainBonus(Location, true);
    
    // 지형 보너스 적용
    int32 TerrainAdjustedStrength = FMath::RoundToInt(static_cast<float>(BaseAttackStrength) * (1.0f + TerrainBonus));
    
    // 체력 비율 배율 적용
    return CalculateActualDamage(TerrainAdjustedStrength, CurrentHealth, MaxHealth);
}

int32 UUnitCombatComponent::CalculateCounterDamage(int32 BaseDefenseStrength, int32 CurrentHealth, int32 MaxHealth) const
{
    // 지형 보너스는 향후 확장 (현재는 0.0f)
    float TerrainBonus = 0.0f; // GetTerrainBonus(Location, false);
    
    // 지형 보너스 적용
    int32 TerrainAdjustedStrength = FMath::RoundToInt(static_cast<float>(BaseDefenseStrength) * (1.0f + TerrainBonus));
    
    // 체력 비율 배율 적용
    return CalculateActualDamage(TerrainAdjustedStrength, CurrentHealth, MaxHealth);
}

int32 UUnitCombatComponent::CalculateActualDamage(int32 BaseDamage, int32 CurrentHealth, int32 MaxHealth) const
{
    // 안전장치: 공격력이 0 이하인 경우
    if (BaseDamage <= 0)
    {
        return 0;
    }

    // 안전장치: 최대 체력이 0 이하인 경우
    if (MaxHealth <= 0)
    {
        return BaseDamage; // 최대 체력이 0이면 배율 무시
    }

    // 체력 비율 계산
    float HealthRatio = static_cast<float>(CurrentHealth) / static_cast<float>(MaxHealth);
    HealthRatio = FMath::Clamp(HealthRatio, 0.0f, 1.0f);

    // 데미지 계산 및 올림 처리
    float DamageFloat = static_cast<float>(BaseDamage) * HealthRatio;
    int32 FinalDamage = FMath::CeilToInt(DamageFloat);

    // 최소 데미지 1 보장
    return FMath::Max(1, FinalDamage);
}

float UUnitCombatComponent::GetTerrainBonus(const FVector& Location, bool bIsAttacker) const
{
    // 향후 World/Grid 시스템과 연동하여 지형 보너스 계산
    // 현재는 기본값 0.0f 반환 (보너스 없음)
    return 0.0f;
}

UUnitStatusComponent* UUnitCombatComponent::GetStatusComponent(AUnitCharacterBase* Unit) const
{
    if (!Unit)
    {
        return nullptr;
    }

    return Unit->GetUnitStatusComponent();
}

