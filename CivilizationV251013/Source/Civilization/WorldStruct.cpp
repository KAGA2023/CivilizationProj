// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldStruct.h"

UWorldTile::UWorldTile()
{
    m_TileData = FTileData();
    m_TileModifiers.Empty();
    bIsSelected = false;
}

bool UWorldTile::IsPassable() const
{
    // 바다는 통과할 수 없음
    if (m_TileData.TerrainType == ETerrainType::Ocean)
    {
        return false;
    }
    
    // 모든 땅 타일은 통과 가능 (산 포함)
    return true;
}

bool UWorldTile::HasResource() const
{
    return m_TileData.ResourceCategory != EResourceCategory::None;
}

FString UWorldTile::GetClimateTypeName() const
{
    return m_TileData.ClimateTypeID.ToString();
}

FString UWorldTile::GetLandTypeName() const
{
    return m_TileData.LandTypeID.ToString();
}


FString UWorldTile::GetBonusResourceName() const
{
    switch (m_TileData.BonusResource)
    {
    case EBonusResource::None:           return TEXT("None");
    case EBonusResource::Wheat:          return TEXT("Wheat");
    case EBonusResource::Rice:           return TEXT("Rice");
    case EBonusResource::Maize:          return TEXT("Maize");
    case EBonusResource::Cattle:         return TEXT("Cattle");
    case EBonusResource::Sheep:          return TEXT("Sheep");
    case EBonusResource::Deer:           return TEXT("Deer");
    case EBonusResource::Copper:         return TEXT("Copper");
    case EBonusResource::Stone:          return TEXT("Stone");
    case EBonusResource::Bananas:        return TEXT("Bananas");
    default:                             return TEXT("Unknown");
    }
}

FString UWorldTile::GetStrategicResourceName() const
{
    switch (m_TileData.StrategicResource)
    {
    case EStrategicResource::None:       return TEXT("None");
    case EStrategicResource::Horses:     return TEXT("Horses");
    case EStrategicResource::Iron:       return TEXT("Iron");
    default:                             return TEXT("Unknown");
    }
}

FString UWorldTile::GetLuxuryResourceName() const
{
    switch (m_TileData.LuxuryResource)
    {
    case ELuxuryResource::None:          return TEXT("None");
    case ELuxuryResource::Furs:          return TEXT("Furs");
    case ELuxuryResource::Diamonds:      return TEXT("Diamonds");
    case ELuxuryResource::Salt:          return TEXT("Salt");
    case ELuxuryResource::Silver:        return TEXT("Silver");
    case ELuxuryResource::Tobacco:       return TEXT("Tobacco");
    case ELuxuryResource::Silk:          return TEXT("Silk");
    case ELuxuryResource::Wine:          return TEXT("Wine");
    case ELuxuryResource::Tea:           return TEXT("Tea");
    case ELuxuryResource::Coffee:        return TEXT("Coffee");
    case ELuxuryResource::Spices:        return TEXT("Spices");
    default:                             return TEXT("Unknown");
    }
}

FString UWorldTile::GetResourceName() const
{
    switch (m_TileData.ResourceCategory)
    {
    case EResourceCategory::Bonus:
        return GetBonusResourceName();
    case EResourceCategory::Strategic:
        return GetStrategicResourceName();
    case EResourceCategory::Luxury:
        return GetLuxuryResourceName();
    default:
        return TEXT("None");
    }
}

FString UWorldTile::GetFullTileName() const
{
    FString FullName = TEXT("");
    
    if (m_TileData.TerrainType == ETerrainType::Land)
    {
        // 땅: 기후 타입 + 땅 타입을 더 자세하게 표시
        FString ClimateName = GetClimateTypeName();
        FString LandName = GetLandTypeName();
        
        FullName = FString::Printf(TEXT("기후: %s\n지형: %s"), *ClimateName, *LandName);
        
        // 숲이 있으면 표시
        if (m_TileData.bHasForest)
        {
            FullName += TEXT("\n특성: 숲");
        }
    }
    else if (m_TileData.TerrainType == ETerrainType::Ocean)
    {
        // 바다: 단순히 "바다"로 표시
        FullName = TEXT("바다");
    }
    
    // 자원이 있으면 추가
    if (HasResource())
    {
        FString ResourceName = GetResourceName();
        if (ResourceName != TEXT("None"))
        {
            FullName = FString::Printf(TEXT("%s\n자원: %s"), *FullName, *ResourceName);
        }
    }
    
    return FullName.IsEmpty() ? TEXT("월드 밖") : FullName;
}

// 타일 모디파이어 시스템 구현
void UWorldTile::AddTileModifier(const FTileModifier& Modifier)
{
    m_TileModifiers.Add(Modifier);
}

void UWorldTile::RemoveTileModifier(const FTileModifier& Modifier)
{
    m_TileModifiers.Remove(Modifier);
}

void UWorldTile::ClearAllModifiers()
{
    m_TileModifiers.Empty();
}

int32 UWorldTile::GetTotalFoodYield() const
{
    int32 TotalFood = m_TileData.CachedFoodYield;
    
    // 모든 모디파이어의 식량 보너스 합산
    for (const FTileModifier& Modifier : m_TileModifiers)
    {
        TotalFood += Modifier.AddFood;
    }
    
    return TotalFood;
}

int32 UWorldTile::GetTotalProductionYield() const
{
    int32 TotalProduction = m_TileData.CachedProductionYield;
    
    // 모든 모디파이어의 생산량 보너스 합산
    for (const FTileModifier& Modifier : m_TileModifiers)
    {
        TotalProduction += Modifier.AddProduction;
    }
    
    return TotalProduction;
}

int32 UWorldTile::GetTotalGoldYield() const
{
    int32 TotalGold = m_TileData.CachedGoldYield;
    
    // 모든 모디파이어의 골드 보너스 합산
    for (const FTileModifier& Modifier : m_TileModifiers)
    {
        TotalGold += Modifier.AddGold;
    }
    
    return TotalGold;
}

int32 UWorldTile::GetTotalScienceYield() const
{
    int32 TotalScience = m_TileData.CachedScienceYield;
    
    // 모든 모디파이어의 과학 보너스 합산
    for (const FTileModifier& Modifier : m_TileModifiers)
    {
        TotalScience += Modifier.AddScience;
    }
    
    return TotalScience;
}

int32 UWorldTile::GetTotalFaithYield() const
{
    int32 TotalFaith = m_TileData.CachedFaithYield;
    
    // 모든 모디파이어의 신앙 보너스 합산
    for (const FTileModifier& Modifier : m_TileModifiers)
    {
        TotalFaith += Modifier.AddFaith;
    }
    
    return TotalFaith;
}


float UWorldTile::GetTotalMovementCost() const
{
    // 기본 이동 비용 (캐시된 값 사용)
    float BaseMovementCost = m_TileData.CachedMovementCost;
    
    // 모든 모디파이어의 이동 비용 배수를 곱함
    float TotalMultiplier = 1.0f;
    for (const FTileModifier& Modifier : m_TileModifiers)
    {
        TotalMultiplier *= Modifier.MovementCostMultiplier;
    }
    
    return BaseMovementCost * TotalMultiplier;
}

int32 UWorldTile::GetTotalDefenseBonus() const
{
    // 기본 방어 보너스 (캐시된 값 사용)
    int32 BaseDefenseBonus = m_TileData.CachedDefenseBonus;
    
    // 모든 모디파이어의 방어 보너스를 더함
    int32 TotalDefenseBonus = BaseDefenseBonus;
    for (const FTileModifier& Modifier : m_TileModifiers)
    {
        TotalDefenseBonus += Modifier.DefenseBonus;
    }
    
    return TotalDefenseBonus;
}
