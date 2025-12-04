// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitCombatComponent.h"
#include "../Status/UnitStatusComponent.h"
#include "../Status/UnitStatusStruct.h"
#include "../Unit/UnitCharacterBase.h"
#include "../World/WorldComponent.h"
#include "../World/WorldStruct.h"
#include "../SuperGameInstance.h"

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

FCombatResult UUnitCombatComponent::ExecuteCombat(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, int32 HexDistance, FVector2D AttackerHex, FVector2D DefenderHex)
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

    // ========== 1단계: 스탯 가져오기 ==========
    int32 BaseAttackerAttackStrength = AttackerStatus->GetAttackStrength();
    int32 BaseDefenderDefenseStrength = DefenderStatus->GetDefenseStrength();
    int32 AttackerCurrentHealth = AttackerStatus->GetCurrentHealth();
    int32 AttackerMaxHealth = AttackerStatus->GetMaxHealth();
    int32 DefenderCurrentHealth = DefenderStatus->GetCurrentHealth();
    int32 DefenderMaxHealth = DefenderStatus->GetMaxHealth();

    // ========== 2단계: 체력 비율 기반 데미지 계산 ==========
    int32 BaseAttackDamage = CalculateAttackDamage(BaseAttackerAttackStrength, AttackerCurrentHealth, AttackerMaxHealth);
    
    int32 BaseCounterDamage = 0;
    if (HexDistance == 1)
    {
        BaseCounterDamage = CalculateCounterDamage(BaseDefenderDefenseStrength, DefenderCurrentHealth, DefenderMaxHealth);
    }

    // ========== 3단계: 지형 보너스 적용 ==========
    int32 AttackerCombatBonus = GetCombatBonusAtHex(AttackerHex);
    int32 DefenderCombatBonus = GetCombatBonusAtHex(DefenderHex);
    
    int32 FinalAttackDamage = BaseAttackDamage + AttackerCombatBonus;
    int32 FinalCounterDamage = (BaseCounterDamage > 0) ? BaseCounterDamage + DefenderCombatBonus : 0;

    // ========== 데미지 적용 (동시에 적용 - 문명6 방식) ==========
    // 피격자가 먼저 죽어도 반격 데미지는 공격자에게 들어감 (거리 1일 때만)
    DefenderStatus->TakeDamage(FinalAttackDamage);
    if (FinalCounterDamage > 0)
    {
        AttackerStatus->TakeDamage(FinalCounterDamage);
    }

    // ========== 결과 확인 ==========
    Result.bDefenderAlive = !DefenderStatus->IsDead();
    Result.bAttackerAlive = !AttackerStatus->IsDead();
    Result.AttackerDamageDealt = FinalAttackDamage;
    Result.DefenderDamageDealt = FinalCounterDamage;
    Result.AttackerDamageTaken = FinalCounterDamage;
    Result.DefenderDamageTaken = FinalAttackDamage;

    // ========== 상태 업데이트 ==========
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
    // 체력 비율 배율 적용 (지형 보너스는 ExecuteCombat에서 별도 적용)
    return CalculateActualDamage(BaseAttackStrength, CurrentHealth, MaxHealth);
}

int32 UUnitCombatComponent::CalculateCounterDamage(int32 BaseDefenseStrength, int32 CurrentHealth, int32 MaxHealth) const
{
    // 체력 비율 배율 적용 (지형 보너스는 ExecuteCombat에서 별도 적용)
    return CalculateActualDamage(BaseDefenseStrength, CurrentHealth, MaxHealth);
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

int32 UUnitCombatComponent::GetCombatBonusAtHex(FVector2D HexPosition) const
{
    // WorldComponent 가져오기
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
            {
                if (UWorldTile* Tile = WorldComponent->GetTileAtHex(HexPosition))
                {
                    // 타일의 전투 보너스 반환 (캐시된 값 + 모디파이어)
                    return Tile->GetTotalCombatBonus();
                }
            }
        }
    }
    
    // WorldComponent를 찾을 수 없으면 0 반환
    return 0;
}

UUnitStatusComponent* UUnitCombatComponent::GetStatusComponent(AUnitCharacterBase* Unit) const
{
    if (!Unit)
    {
        return nullptr;
    }

    return Unit->GetUnitStatusComponent();
}

