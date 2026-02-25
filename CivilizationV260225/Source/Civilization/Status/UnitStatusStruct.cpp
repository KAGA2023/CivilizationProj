// Fill out your copyright notice in the Description page of Project Settings.

#include "UnitStatusStruct.h"

// FUnitStatModifier 연산자 오버로드
bool FUnitStatModifier::operator==(const FUnitStatModifier& Other) const
{
    return AddHealth == Other.AddHealth &&
           AddAttackStrength == Other.AddAttackStrength &&
           AddDefenseStrength == Other.AddDefenseStrength &&
           AddMovementPoints == Other.AddMovementPoints &&
           AddRange == Other.AddRange;
}

FUnitStatModifier FUnitStatModifier::operator+(const FUnitStatModifier& Other) const
{
    FUnitStatModifier Result;
    Result.AddHealth = AddHealth + Other.AddHealth;
    Result.AddAttackStrength = AddAttackStrength + Other.AddAttackStrength;
    Result.AddDefenseStrength = AddDefenseStrength + Other.AddDefenseStrength;
    Result.AddMovementPoints = AddMovementPoints + Other.AddMovementPoints;
    Result.AddRange = AddRange + Other.AddRange;
    return Result;
}

FUnitStatModifier FUnitStatModifier::operator-(const FUnitStatModifier& Other) const
{
    FUnitStatModifier Result;
    Result.AddHealth = AddHealth - Other.AddHealth;
    Result.AddAttackStrength = AddAttackStrength - Other.AddAttackStrength;
    Result.AddDefenseStrength = AddDefenseStrength - Other.AddDefenseStrength;
    Result.AddMovementPoints = AddMovementPoints - Other.AddMovementPoints;
    Result.AddRange = AddRange - Other.AddRange;
    return Result;
}

void FUnitStatModifier::operator+=(const FUnitStatModifier& Other)
{
    AddHealth += Other.AddHealth;
    AddAttackStrength += Other.AddAttackStrength;
    AddDefenseStrength += Other.AddDefenseStrength;
    AddMovementPoints += Other.AddMovementPoints;
    AddRange += Other.AddRange;
}

void FUnitStatModifier::operator-=(const FUnitStatModifier& Other)
{
    AddHealth -= Other.AddHealth;
    AddAttackStrength -= Other.AddAttackStrength;
    AddDefenseStrength -= Other.AddDefenseStrength;
    AddMovementPoints -= Other.AddMovementPoints;
    AddRange -= Other.AddRange;
}

void FUnitStatModifier::Reset()
{
    AddHealth = 0;
    AddAttackStrength = 0;
    AddDefenseStrength = 0;
    AddMovementPoints = 0;
    AddRange = 0;
}
