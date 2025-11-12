// Fill out your copyright notice in the Description Settings.

#include "UnitStatusComponent.h"
#include "Engine/DataTable.h"

UUnitStatusComponent::UUnitStatusComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UUnitStatusComponent::InitFromBaseStat(const FUnitBaseStat& InBaseStat)
{
    m_BaseStat = InBaseStat;
    
    // 현재 상태 초기화
    m_CurrentStat.RemainingHealth = m_BaseStat.MaxHealth;
    m_CurrentStat.HasAttacked = false;
    m_CurrentStat.IsWait = false;
    m_CurrentStat.IsAlert = false;
    m_CurrentStat.IsSleep = false;

    // 최종 스테이터스 계산 (모디파이어 적용)
    RecalculateStats();
    
    // 최종 스테이터스 계산 후, 현재 이동력을 최종 이동력으로 설정
    m_CurrentStat.RemainingMovementPoints = m_FinalStat.MovementPoints;
}

void UUnitStatusComponent::InitializeFromRowName(FName RowName)
{
    if (!UnitStatusTable)
    {
        LoadUnitStatusTable();
        
        // 다시 확인
        if (!UnitStatusTable)
        {
            return;
        }
    }

    // RowName으로 직접 데이터 로딩
    FUnitBaseStat* UnitStat = UnitStatusTable->FindRow<FUnitBaseStat>(RowName, TEXT("InitializeFromRowName"));
    if (!UnitStat)
    {
        return;
    }

    // 스테이터스 초기화
    InitFromBaseStat(*UnitStat);
}

void UUnitStatusComponent::LoadUnitStatusTable()
{
    // SoftObjectPath를 사용한 로딩
    FSoftObjectPath UnitStatusTablePath(TEXT("/Game/Civilization/Data/DT_UnitBaseStat.DT_UnitBaseStat"));
    UnitStatusTable = Cast<UDataTable>(UnitStatusTablePath.TryLoad());
}

void UUnitStatusComponent::AddStatModifier(const FUnitStatModifier& Modifier)
{
    m_StatModifiers.Add(Modifier);
    RecalculateStats();
}

void UUnitStatusComponent::RemoveStatModifier(const FUnitStatModifier& Modifier)
{
    m_StatModifiers.Remove(Modifier);
    RecalculateStats();
}

void UUnitStatusComponent::ClearAllModifiers()
{
    m_StatModifiers.Empty();
    RecalculateStats();
}

void UUnitStatusComponent::RecalculateStats()
{
    FUnitStatModifier TotalModifier = CalculateTotalModifier();
    
    // 최종 스테이터스 계산 (컴포넌트에서 직접 처리)
    m_FinalStat.AttackStrength = FMath::Max(0, m_BaseStat.AttackStrength + TotalModifier.AddAttackStrength);
    m_FinalStat.DefenseStrength = FMath::Max(0, m_BaseStat.DefenseStrength + TotalModifier.AddDefenseStrength);
    m_FinalStat.MovementPoints = FMath::Max(1, m_BaseStat.MovementPoints + TotalModifier.AddMovementPoints);
    m_FinalStat.SightRange = FMath::Max(1, m_BaseStat.SightRange + TotalModifier.AddSightRange);
    m_FinalStat.Range = FMath::Max(0, m_BaseStat.Range + TotalModifier.AddRange);
    m_FinalStat.GoldCost = FMath::Max(0, m_BaseStat.GoldCost + TotalModifier.AddGoldCost);
    m_FinalStat.FoodCost = FMath::Max(0, m_BaseStat.FoodCost + TotalModifier.AddFoodCost);
    
    // 현재 체력이 최대치를 초과하지 않도록 조정
    int32 NewMaxHealth = GetMaxHealth();
    if (m_CurrentStat.RemainingHealth > NewMaxHealth)
    {
        m_CurrentStat.RemainingHealth = NewMaxHealth;
    }
    
    // 현재 이동력이 최대치를 초과하지 않도록 조정
    if (m_CurrentStat.RemainingMovementPoints > m_FinalStat.MovementPoints)
    {
        m_CurrentStat.RemainingMovementPoints = m_FinalStat.MovementPoints;
    }
}

FUnitStatModifier UUnitStatusComponent::CalculateTotalModifier() const
{
    FUnitStatModifier TotalModifier;
    TotalModifier.Reset();

    for (const auto& Modifier : m_StatModifiers)
    {
        TotalModifier += Modifier;
    }

    return TotalModifier;
}

void UUnitStatusComponent::ResetTurn()
{
    // 턴 초기화시 최종 이동력으로 리셋 (모디파이어 반영된 값)
    m_CurrentStat.RemainingMovementPoints = m_FinalStat.MovementPoints;
    m_CurrentStat.HasAttacked = false;
}

void UUnitStatusComponent::ConsumeMovement(int32 Amount)
{
    m_CurrentStat.RemainingMovementPoints = FMath::Max(0, m_CurrentStat.RemainingMovementPoints - Amount);
}

bool UUnitStatusComponent::CanMove() const
{
    return m_CurrentStat.RemainingMovementPoints > 0 && !m_CurrentStat.HasAttacked;
}

bool UUnitStatusComponent::CanAttack() const
{
    return !m_CurrentStat.HasAttacked;
}

void UUnitStatusComponent::SetWait(bool bWait)
{
    m_CurrentStat.IsWait = bWait;
    if (bWait)
    {
        m_CurrentStat.IsAlert = false;
        m_CurrentStat.IsSleep = false;
    }
}

void UUnitStatusComponent::SetAlert(bool bAlert)
{
    m_CurrentStat.IsAlert = bAlert;
    if (bAlert)
    {
        m_CurrentStat.IsWait = false;
        m_CurrentStat.IsSleep = false;
    }
}

void UUnitStatusComponent::SetSleep(bool bSleep)
{
    m_CurrentStat.IsSleep = bSleep;
    if (bSleep)
    {
        m_CurrentStat.IsWait = false;
        m_CurrentStat.IsAlert = false;
    }
}

void UUnitStatusComponent::TakeDamage(int32 DamageAmount)
{
    if (m_CurrentStat.RemainingHealth <= 0) return;

    m_CurrentStat.RemainingHealth -= DamageAmount;
    m_CurrentStat.RemainingHealth = FMath::Max(0, m_CurrentStat.RemainingHealth);

    if (m_CurrentStat.RemainingHealth <= 0)
    {
        OnUnitDeath();
    }
}

void UUnitStatusComponent::Heal(int32 HealAmount)
{
    int32 MaxHealth = GetMaxHealth();
    m_CurrentStat.RemainingHealth += HealAmount;
    m_CurrentStat.RemainingHealth = FMath::Min(MaxHealth, m_CurrentStat.RemainingHealth);
}

int32 UUnitStatusComponent::GetMaxHealth() const
{
    FUnitStatModifier TotalModifier = CalculateTotalModifier();
    return FMath::Max(1, m_BaseStat.MaxHealth + TotalModifier.AddHealth);
}

bool UUnitStatusComponent::IsDead() const
{
    return m_CurrentStat.RemainingHealth <= 0;
}

bool UUnitStatusComponent::IsValid() const
{
    return m_BaseStat.UnitClass != EUnitClass::None;
}

void UUnitStatusComponent::OnUnitDeath()
{
    // 죽음 이벤트 브로드캐스트
    if (AActor* Owner = GetOwner())
    {
        OnUnitDied.Broadcast(Owner);
    }
}

// Called when the game starts
void UUnitStatusComponent::BeginPlay()
{
    Super::BeginPlay();
}

// Called every frame
void UUnitStatusComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 필요시 턴제 게임 로직 처리
    // 예: 자동 회복, 상태 효과 등
}
