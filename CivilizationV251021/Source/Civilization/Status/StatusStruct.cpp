// Fill out your copyright notice in the Description page of Project Settings.

#include "StatusStruct.h"

// FUnitStatModifier 연산자 오버로드
bool FUnitStatModifier::operator==(const FUnitStatModifier& Other) const
{
    return Health == Other.Health &&
           AttackStrength == Other.AttackStrength &&
           DefenseStrength == Other.DefenseStrength &&
           MovementPoints == Other.MovementPoints &&
           SightRange == Other.SightRange &&
           Range == Other.Range &&
           ProductionCost == Other.ProductionCost &&
           GoldCost == Other.GoldCost &&
           FaithCost == Other.FaithCost &&
           MaintenanceCost == Other.MaintenanceCost;
}

FUnitStatModifier FUnitStatModifier::operator+(const FUnitStatModifier& Other) const
{
    FUnitStatModifier Result;
    Result.Health = Health + Other.Health;
    Result.AttackStrength = AttackStrength + Other.AttackStrength;
    Result.DefenseStrength = DefenseStrength + Other.DefenseStrength;
    Result.MovementPoints = MovementPoints + Other.MovementPoints;
    Result.SightRange = SightRange + Other.SightRange;
    Result.Range = Range + Other.Range;
    Result.ProductionCost = ProductionCost + Other.ProductionCost;
    Result.GoldCost = GoldCost + Other.GoldCost;
    Result.FaithCost = FaithCost + Other.FaithCost;
    Result.MaintenanceCost = MaintenanceCost + Other.MaintenanceCost;
    return Result;
}

FUnitStatModifier FUnitStatModifier::operator-(const FUnitStatModifier& Other) const
{
    FUnitStatModifier Result;
    Result.Health = Health - Other.Health;
    Result.AttackStrength = AttackStrength - Other.AttackStrength;
    Result.DefenseStrength = DefenseStrength - Other.DefenseStrength;
    Result.MovementPoints = MovementPoints - Other.MovementPoints;
    Result.SightRange = SightRange - Other.SightRange;
    Result.Range = Range - Other.Range;
    Result.ProductionCost = ProductionCost - Other.ProductionCost;
    Result.GoldCost = GoldCost - Other.GoldCost;
    Result.FaithCost = FaithCost - Other.FaithCost;
    Result.MaintenanceCost = MaintenanceCost - Other.MaintenanceCost;
    return Result;
}

void FUnitStatModifier::operator+=(const FUnitStatModifier& Other)
{
    Health += Other.Health;
    AttackStrength += Other.AttackStrength;
    DefenseStrength += Other.DefenseStrength;
    MovementPoints += Other.MovementPoints;
    SightRange += Other.SightRange;
    Range += Other.Range;
    ProductionCost += Other.ProductionCost;
    GoldCost += Other.GoldCost;
    FaithCost += Other.FaithCost;
    MaintenanceCost += Other.MaintenanceCost;
}

void FUnitStatModifier::operator-=(const FUnitStatModifier& Other)
{
    Health -= Other.Health;
    AttackStrength -= Other.AttackStrength;
    DefenseStrength -= Other.DefenseStrength;
    MovementPoints -= Other.MovementPoints;
    SightRange -= Other.SightRange;
    Range -= Other.Range;
    ProductionCost -= Other.ProductionCost;
    GoldCost -= Other.GoldCost;
    FaithCost -= Other.FaithCost;
    MaintenanceCost -= Other.MaintenanceCost;
}

void FUnitStatModifier::Reset()
{
    Health = 0;
    AttackStrength = 0;
    DefenseStrength = 0;
    MovementPoints = 0;
    SightRange = 0;
    Range = 0;
    ProductionCost = 0;
    GoldCost = 0;
    FaithCost = 0;
    MaintenanceCost = 0;
}

// FUnitCurrentStat 유틸리티 함수들
void FUnitCurrentStat::ResetTurn()
{
    RemainingMovementPoints = CurrentMovementPoints;
    HasAttacked = false;
    HasMoved = false;
}

void FUnitCurrentStat::ConsumeMovement(int32 Amount)
{
    RemainingMovementPoints = FMath::Max(0, RemainingMovementPoints - Amount);
    if (RemainingMovementPoints > 0)
    {
        HasMoved = true;
    }
}

bool FUnitCurrentStat::CanMove() const
{
    return RemainingMovementPoints > 0 && !HasAttacked && !IsSleep;
}

bool FUnitCurrentStat::CanAttack() const
{
    return !HasAttacked && !IsSleep;
}

void FUnitCurrentStat::SetFortified(bool bFortified)
{
    IsFortified = bFortified;
    if (bFortified)
    {
        IsAlert = false;
        IsSleep = false;
        FortificationTurns++;
    }
    else
    {
        FortificationTurns = 0;
    }
}

void FUnitCurrentStat::SetAlert(bool bAlert)
{
    IsAlert = bAlert;
    if (bAlert)
    {
        IsFortified = false;
        IsSleep = false;
        FortificationTurns = 0;
    }
}

void FUnitCurrentStat::SetSleep(bool bSleep)
{
    IsSleep = bSleep;
    if (bSleep)
    {
        IsFortified = false;
        IsAlert = false;
        FortificationTurns = 0;
    }
}

void FUnitCurrentStat::AddPromotion(FName PromotionName)
{
    if (!ActivePromotions.Contains(PromotionName))
    {
        ActivePromotions.Add(PromotionName);
    }
}

void FUnitCurrentStat::RemovePromotion(FName PromotionName)
{
    ActivePromotions.Remove(PromotionName);
}

bool FUnitCurrentStat::HasPromotion(FName PromotionName) const
{
    return ActivePromotions.Contains(PromotionName);
}

void FUnitCurrentStat::AddAbility(FName AbilityName)
{
    if (!ActiveAbilities.Contains(AbilityName))
    {
        ActiveAbilities.Add(AbilityName);
    }
}

void FUnitCurrentStat::RemoveAbility(FName AbilityName)
{
    ActiveAbilities.Remove(AbilityName);
}

bool FUnitCurrentStat::HasAbility(FName AbilityName) const
{
    return ActiveAbilities.Contains(AbilityName);
}
