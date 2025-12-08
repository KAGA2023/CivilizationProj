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

    // 1. 전투 전 검증 (Hex 좌표 포함하여 사거리/층수 검증)
    if (!CanExecuteCombat(Attacker, Defender, AttackerHex, DefenderHex))
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
    // 반격 조건: 거리 1 AND 층수 차이 1 이하
    if (HexDistance == 1)
    {
        // 기존 GetFloorLevelAtHex() 함수 재활용
        int32 AttackerFloor = GetFloorLevelAtHex(AttackerHex);
        int32 DefenderFloor = GetFloorLevelAtHex(DefenderHex);
        int32 FloorDifference = FMath::Abs(AttackerFloor - DefenderFloor);
        
        // 층수 차이가 1 이하일 때만 반격
        if (FloorDifference <= 1)
        {
            BaseCounterDamage = CalculateCounterDamage(BaseDefenderDefenseStrength, DefenderCurrentHealth, DefenderMaxHealth);
        }
    }

    // ========== 3단계: 지형 보너스 적용 ==========
    int32 AttackerRange = AttackerStatus->GetRange();
    int32 DefenderRange = DefenderStatus->GetRange();

    int32 AttackerCombatBonus = 0;
    int32 DefenderCombatBonus = 0;

    // 공격자 보너스 계산
    if (AttackerRange > 1)
    {
        // 원거리 유닛: Range 보너스 적용 (사거리 체크에만 사용, 공격력에는 영향 없음)
        // Range 보너스는 HandleCombatSelection에서 사거리 체크 시 적용
        AttackerCombatBonus = 0; // 원거리 유닛은 공격력 보너스 없음
    }
    else
    {
        // 근접 유닛: 기존 공격력 보너스 적용
        AttackerCombatBonus = CalculateCombatBonus(AttackerHex, DefenderHex);
    }

    // 방어자 보너스 계산 (반격용)
    if (DefenderRange > 1)
    {
        // 원거리 유닛: Range 보너스 적용
        DefenderCombatBonus = 0; // 원거리 유닛은 공격력 보너스 없음
    }
    else
    {
        // 근접 유닛: 기존 공격력 보너스 적용
        DefenderCombatBonus = CalculateCombatBonus(DefenderHex, AttackerHex);
    }
    
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

bool UUnitCombatComponent::CanExecuteCombat(AUnitCharacterBase* Attacker, AUnitCharacterBase* Defender, FVector2D AttackerHex, FVector2D DefenderHex) const
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

    // 6. Hex 파라미터가 제공된 경우 사거리/층수 검증 추가
    if (AttackerHex != FVector2D::ZeroVector && DefenderHex != FVector2D::ZeroVector)
    {
        // WorldComponent 가져오기
        UWorld* World = GetWorld();
        if (!World)
        {
            return false;
        }

        USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance());
        if (!SuperGameInst)
        {
            return false;
        }

        UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent();
        if (!WorldComponent)
        {
            return false;
        }

        // 사거리 검증
        int32 BaseAttackRange = AttackerStatus->GetRange();
        int32 AttackRange = BaseAttackRange;

        // 원거리 유닛인 경우 Range 보너스 적용
        if (BaseAttackRange > 1)
        {
            int32 RangeBonus = CalculateRangeBonus(AttackerHex);
            AttackRange += RangeBonus;
        }

        int32 HexDistance = WorldComponent->GetHexDistance(AttackerHex, DefenderHex);

        // 사거리 밖이면 공격 불가
        if (HexDistance > AttackRange)
        {
            return false;
        }

        // Range == 1인 근접 공격일 때만 층수 차이 체크
        if (BaseAttackRange == 1)
        {
            int32 AttackerFloor = GetFloorLevelAtHex(AttackerHex);
            int32 DefenderFloor = GetFloorLevelAtHex(DefenderHex);
            int32 FloorDifference = FMath::Abs(AttackerFloor - DefenderFloor);

            // 층수 차이가 2 이상이면 공격 불가
            if (FloorDifference >= 2)
            {
                return false;
            }
        }
    }

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

// 지형 보너스 계산 (층수 + 숲)
int32 UUnitCombatComponent::CalculateCombatBonus(FVector2D MyHex, FVector2D EnemyHex) const
{
    int32 TotalBonus = 0;
    
    // 1. 층수 보너스
    TotalBonus += CalculateHeightBonus(MyHex, EnemyHex);
    
    // 2. 숲 보너스
    TotalBonus += CalculateForestBonus(MyHex);
    
    return TotalBonus;
}

// 층수 보너스 계산
int32 UUnitCombatComponent::CalculateHeightBonus(FVector2D MyHex, FVector2D EnemyHex) const
{
    int32 MyFloor = GetFloorLevelAtHex(MyHex);
    int32 EnemyFloor = GetFloorLevelAtHex(EnemyHex);
    
    // 내 타일이 상대 타일보다 높으면 보너스
    if (MyFloor > EnemyFloor)
    {
        return LAND_ATK_BONUS;
    }
    
    return 0;
}

// 숲 보너스 계산
int32 UUnitCombatComponent::CalculateForestBonus(FVector2D MyHex) const
{
    // WorldComponent 가져오기
    if (UWorld* World = GetWorld())
    {
        if (USuperGameInstance* SuperGameInst = Cast<USuperGameInstance>(World->GetGameInstance()))
        {
            if (UWorldComponent* WorldComponent = SuperGameInst->GetGeneratedWorldComponent())
            {
                if (UWorldTile* Tile = WorldComponent->GetTileAtHex(MyHex))
                {
                    // 바다 타일 체크 (통행 불가이므로 체크 불필요하지만 안전장치)
                    if (Tile->GetTerrainType() == ETerrainType::Ocean)
                    {
                        return 0;
                    }
                    
                    // 숲이 있으면 보너스
                    if (Tile->HasForest())
                    {
                        return LAND_ATK_BONUS;
                    }
                }
            }
        }
    }
    
    return 0;
}

// 타일의 층수 가져오기
int32 UUnitCombatComponent::GetFloorLevelAtHex(FVector2D HexPosition) const
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
                    // 바다 타일은 체크하지 않음 (통행 불가)
                    if (Tile->GetTerrainType() == ETerrainType::Ocean)
                    {
                        return 1; // 기본값 반환 (실제로는 사용되지 않음)
                    }
                    
                    // UnitManager의 GetFloorLevel 로직 재현
                    ELandType LandType = Tile->GetLandType();
                    switch (LandType)
                    {
                    case ELandType::Plains:
                        return 1;
                    case ELandType::Hills:
                        return 2;
                    case ELandType::Mountains:
                        return 3;
                    default:
                        return 1;
                    }
                }
            }
        }
    }
    
    return 1; // 기본값
}

// 원거리 유닛의 Range 보너스 계산 (지형 기반)
int32 UUnitCombatComponent::CalculateRangeBonus(FVector2D MyHex) const
{
    int32 FloorLevel = GetFloorLevelAtHex(MyHex);
    
    // 평지(1층): +0, 언덕(2층): +1, 산(3층): +2
    return FloorLevel - 1;
}

UUnitStatusComponent* UUnitCombatComponent::GetStatusComponent(AUnitCharacterBase* Unit) const
{
    if (!Unit)
    {
        return nullptr;
    }

    return Unit->GetUnitStatusComponent();
}

// 지형 보너스 텍스트 생성 (UI용)
FText UUnitCombatComponent::GetTerrainBonusText(FVector2D MyHex, FVector2D EnemyHex, int32 UnitRange) const
{
    FString ResultText;
    
    // 층수 보너스와 숲 보너스는 Range가 1 이하인 근접 유닛만
    if (UnitRange <= 1)
    {
        // 1. 층수 보너스 텍스트
        int32 HeightBonus = CalculateHeightBonus(MyHex, EnemyHex);
        
        if (HeightBonus > 0)
        {
            ResultText += FString::Printf(TEXT("고지대 +%d"), HeightBonus);
        }
        
        // 2. 숲 보너스 텍스트
        int32 ForestBonus = CalculateForestBonus(MyHex);
        
        if (HeightBonus > 0)
        {
            ResultText += TEXT("\n");
        }
        if (ForestBonus > 0)
        {
            ResultText += FString::Printf(TEXT("숲지대 +%d"), ForestBonus);
        }
    }
    
    // 3. 사거리 보너스 텍스트 (Range가 2 이상인 원거리 유닛만)
    if (UnitRange > 1)
    {
        int32 RangeBonus = CalculateRangeBonus(MyHex);
        
        if (RangeBonus > 0)
        {
            if (!ResultText.IsEmpty())
            {
                ResultText += TEXT("\n");
            }
            ResultText += FString::Printf(TEXT("사거리 +%d"), RangeBonus);
        }
    }
    
    return FText::FromString(ResultText);
}

