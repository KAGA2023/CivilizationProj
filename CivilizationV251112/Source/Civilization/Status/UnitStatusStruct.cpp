// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitStatusStruct.h"

// FUnitStatModifier 연산자 오버로드
bool FUnitStatModifier::operator==(const FUnitStatModifier& Other) const
{
    return AddHealth == Other.AddHealth &&
           AddAttackStrength == Other.AddAttackStrength &&
           AddDefenseStrength == Other.AddDefenseStrength &&
           AddMovementPoints == Other.AddMovementPoints &&
           AddSightRange == Other.AddSightRange &&
           AddRange == Other.AddRange &&
           AddGoldCost == Other.AddGoldCost &&
           AddFoodCost == Other.AddFoodCost;
}

FUnitStatModifier FUnitStatModifier::operator+(const FUnitStatModifier& Other) const
{
    FUnitStatModifier Result;
    Result.AddHealth = AddHealth + Other.AddHealth;
    Result.AddAttackStrength = AddAttackStrength + Other.AddAttackStrength;
    Result.AddDefenseStrength = AddDefenseStrength + Other.AddDefenseStrength;
    Result.AddMovementPoints = AddMovementPoints + Other.AddMovementPoints;
    Result.AddSightRange = AddSightRange + Other.AddSightRange;
    Result.AddRange = AddRange + Other.AddRange;
    Result.AddGoldCost = AddGoldCost + Other.AddGoldCost;
    Result.AddFoodCost = AddFoodCost + Other.AddFoodCost;
    return Result;
}

FUnitStatModifier FUnitStatModifier::operator-(const FUnitStatModifier& Other) const
{
    FUnitStatModifier Result;
    Result.AddHealth = AddHealth - Other.AddHealth;
    Result.AddAttackStrength = AddAttackStrength - Other.AddAttackStrength;
    Result.AddDefenseStrength = AddDefenseStrength - Other.AddDefenseStrength;
    Result.AddMovementPoints = AddMovementPoints - Other.AddMovementPoints;
    Result.AddSightRange = AddSightRange - Other.AddSightRange;
    Result.AddRange = AddRange - Other.AddRange;
    Result.AddGoldCost = AddGoldCost - Other.AddGoldCost;
    Result.AddFoodCost = AddFoodCost - Other.AddFoodCost;
    return Result;
}

void FUnitStatModifier::operator+=(const FUnitStatModifier& Other)
{
    AddHealth += Other.AddHealth;
    AddAttackStrength += Other.AddAttackStrength;
    AddDefenseStrength += Other.AddDefenseStrength;
    AddMovementPoints += Other.AddMovementPoints;
    AddSightRange += Other.AddSightRange;
    AddRange += Other.AddRange;
    AddGoldCost += Other.AddGoldCost;
    AddFoodCost += Other.AddFoodCost;
}

void FUnitStatModifier::operator-=(const FUnitStatModifier& Other)
{
    AddHealth -= Other.AddHealth;
    AddAttackStrength -= Other.AddAttackStrength;
    AddDefenseStrength -= Other.AddDefenseStrength;
    AddMovementPoints -= Other.AddMovementPoints;
    AddSightRange -= Other.AddSightRange;
    AddRange -= Other.AddRange;
    AddGoldCost -= Other.AddGoldCost;
    AddFoodCost -= Other.AddFoodCost;
}

void FUnitStatModifier::Reset()
{
    AddHealth = 0;
    AddAttackStrength = 0;
    AddDefenseStrength = 0;
    AddMovementPoints = 0;
    AddSightRange = 0;
    AddRange = 0;
    AddGoldCost = 0;
    AddFoodCost = 0;
}
