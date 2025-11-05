// Fill out your copyright notice in the Description Settings.

#include "CityComponent.h"
#include "Engine/DataTable.h"
#include "../Status/UnitStatusStruct.h"

UCityComponent::UCityComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCityComponent::BeginPlay()
{
    Super::BeginPlay();

    // 데이터 테이블 로드
    LoadBuildingDataTable();
    LoadUnitStatusTable();
}

void UCityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // 필요시 도시 로직 처리
    // 예: 자동 회복 등
}

void UCityComponent::InitFromCityData(const FCityData& InCityData)
{
    m_CityData = InCityData;

    // 현재 상태 초기화
    m_CurrentStat.CurrentlyProducing = EBuildingType::None;
    m_CurrentStat.ProductionProgress = 0;
    m_CurrentStat.ProductionCost = 0;
    m_CurrentStat.FoodProgress = 0;
    m_CurrentStat.FoodCost = 0;
    m_CurrentStat.bIsProducingUnit = false;
    m_CurrentStat.ProducingUnitName = NAME_None;
    m_CurrentStat.UnitProductionProgress = 0;
    m_CurrentStat.UnitProductionCost = 0;
    m_CurrentStat.UnitFoodProgress = 0;
    m_CurrentStat.UnitFoodCost = 0;

    // 생산량 재계산
    RecalculateYields();

    // 최종 체력으로 현재 체력 설정
    m_CurrentStat.RemainingHealth = m_FinalStat.MaxHealth;
}

void UCityComponent::InitializeFromDataTable(FName RowName)
{
    // 데이터 테이블 로드
    if (!BuildingDataTable)
    {
        LoadBuildingDataTable();
    }

    // 데이터 테이블에서 도시 데이터 찾기 (필요시 구현)
    // 현재는 InitFromCityData 사용
}

void UCityComponent::LoadBuildingDataTable()
{
    // SoftObjectPath를 사용한 로딩
    FSoftObjectPath BuildingDataTablePath(TEXT("/Game/Civilization/Data/DT_BuildingData.DT_BuildingData"));
    BuildingDataTable = Cast<UDataTable>(BuildingDataTablePath.TryLoad());
}

void UCityComponent::LoadUnitStatusTable()
{
    // SoftObjectPath를 사용한 로딩
    FSoftObjectPath UnitStatusTablePath(TEXT("/Game/Civilization/Data/DT_UnitBaseStat.DT_UnitBaseStat"));
    UnitStatusTable = Cast<UDataTable>(UnitStatusTablePath.TryLoad());
}

void UCityComponent::RecalculateYields()
{
    // 기본 생산량 시작
    m_FinalStat.FoodYield = m_CityData.FoodYield;
    m_FinalStat.ProductionYield = m_CityData.ProductionYield;
    m_FinalStat.GoldYield = m_CityData.GoldYield;
    m_FinalStat.ScienceYield = m_CityData.ScienceYield;
    m_FinalStat.FaithYield = m_CityData.FaithYield;
    m_FinalStat.MaxHealth = m_CityData.MaxHealth;

    // 모든 건물의 생산량 합산
    for (const EBuildingType& BuildingType : m_CityData.BuiltBuildings)
    {
        FBuildingData BuildingData = GetBuildingDataFromTable(BuildingType);
        m_FinalStat.FoodYield += BuildingData.FoodYield;
        m_FinalStat.ProductionYield += BuildingData.ProductionYield;
        m_FinalStat.GoldYield += BuildingData.GoldYield;
        m_FinalStat.ScienceYield += BuildingData.ScienceYield;
        m_FinalStat.FaithYield += BuildingData.FaithYield;
        m_FinalStat.MaxHealth += BuildingData.MaxHealth;
    }
}

void UCityComponent::AddBuilding(EBuildingType BuildingType)
{
    if (BuildingType == EBuildingType::None || HasBuilding(BuildingType))
    {
        return;
    }

    m_CityData.BuiltBuildings.Add(BuildingType);

    // 생산량 재계산
    RecalculateYields();
}

void UCityComponent::RemoveBuilding(EBuildingType BuildingType)
{
    if (!HasBuilding(BuildingType))
    {
        return;
    }

    m_CityData.BuiltBuildings.Remove(BuildingType);

    // 생산량 재계산
    RecalculateYields();
}

bool UCityComponent::HasBuilding(EBuildingType BuildingType) const
{
    return m_CityData.BuiltBuildings.Contains(BuildingType);
}

FBuildingData UCityComponent::GetBuildingData(EBuildingType BuildingType) const
{
    return GetBuildingDataFromTable(BuildingType);
}

FBuildingData UCityComponent::GetBuildingDataFromTable(EBuildingType BuildingType) const
{
    if (!BuildingDataTable)
    {
        return FBuildingData();
    }

    // BuildingType을 FName으로 변환
    FString BuildingTypeName = UEnum::GetDisplayValueAsText(BuildingType).ToString();
    FName BuildingRowName = FName(*BuildingTypeName);

    FBuildingData* BuildingData = BuildingDataTable->FindRow<FBuildingData>(BuildingRowName, TEXT("GetBuildingDataFromTable"));
    if (BuildingData)
    {
        return *BuildingData;
    }

    return FBuildingData();
}

int32 UCityComponent::GetFinalFoodYield() const
{
    return m_FinalStat.FoodYield;
}

int32 UCityComponent::GetFinalProductionYield() const
{
    return m_FinalStat.ProductionYield;
}

int32 UCityComponent::GetFinalGoldYield() const
{
    return m_FinalStat.GoldYield;
}

int32 UCityComponent::GetFinalScienceYield() const
{
    return m_FinalStat.ScienceYield;
}

int32 UCityComponent::GetFinalFaithYield() const
{
    return m_FinalStat.FaithYield;
}

int32 UCityComponent::GetFinalMaxHealth() const
{
    return m_FinalStat.MaxHealth;
}

void UCityComponent::StartBuildingProduction(EBuildingType BuildingType)
{
    if (BuildingType == EBuildingType::None)
    {
        return;
    }

    // 이미 건설된 건물이면 생산 불가
    if (HasBuilding(BuildingType))
    {
        return;
    }

    // 건물 데이터에서 비용 가져오기
    FBuildingData BuildingData = GetBuildingData(BuildingType);
    
    m_CurrentStat.CurrentlyProducing = BuildingType;
    m_CurrentStat.ProductionProgress = 0;
    m_CurrentStat.ProductionCost = BuildingData.ProductionCost;
    m_CurrentStat.FoodProgress = 0;
    m_CurrentStat.FoodCost = BuildingData.FoodCost;
}

void UCityComponent::ChangeBuildingProduction(EBuildingType NewBuildingType)
{
    if (NewBuildingType == EBuildingType::None)
    {
        return;
    }

    // 이미 건설된 건물이면 생산 불가
    if (HasBuilding(NewBuildingType))
    {
        return;
    }

    // 건물 데이터에서 비용 가져오기
    FBuildingData BuildingData = GetBuildingData(NewBuildingType);
    
    // 유닛 생산 상태 리셋 (건물 생산으로 전환하므로)
    m_CurrentStat.bIsProducingUnit = false;
    m_CurrentStat.ProducingUnitName = NAME_None;
    m_CurrentStat.UnitProductionProgress = 0;
    m_CurrentStat.UnitProductionCost = 0;
    m_CurrentStat.UnitFoodProgress = 0;
    m_CurrentStat.UnitFoodCost = 0;
    
    // 건물 생산 설정 (진행도 초기화)
    m_CurrentStat.CurrentlyProducing = NewBuildingType;
    m_CurrentStat.ProductionProgress = 0;
    m_CurrentStat.ProductionCost = BuildingData.ProductionCost;
    m_CurrentStat.FoodProgress = 0;
    m_CurrentStat.FoodCost = BuildingData.FoodCost;
}

void UCityComponent::UpdateBuildingProductionProgress(int32 FoodAmount, int32 ProductionAmount)
{
    if (m_CurrentStat.CurrentlyProducing == EBuildingType::None)
    {
        return;
    }

    // 진행도 업데이트 (목표치를 넘지 않도록)
    if (m_CurrentStat.FoodCost > 0)
    {
        m_CurrentStat.FoodProgress += FoodAmount;
        m_CurrentStat.FoodProgress = FMath::Min(m_CurrentStat.FoodProgress, m_CurrentStat.FoodCost);
    }
    
    if (m_CurrentStat.ProductionCost > 0)
    {
        m_CurrentStat.ProductionProgress += ProductionAmount;
        m_CurrentStat.ProductionProgress = FMath::Min(m_CurrentStat.ProductionProgress, m_CurrentStat.ProductionCost);
    }

    // 완료 조건 확인 및 완료 처리
    CompleteBuildingProduction();
}

EBuildingType UCityComponent::CompleteBuildingProduction()
{
    if (m_CurrentStat.CurrentlyProducing == EBuildingType::None)
    {
        return EBuildingType::None;
    }

    // 완료 조건 확인
    bool bFoodComplete = (m_CurrentStat.FoodCost <= 0 || m_CurrentStat.FoodProgress >= m_CurrentStat.FoodCost);
    bool bProductionComplete = (m_CurrentStat.ProductionCost <= 0 || m_CurrentStat.ProductionProgress >= m_CurrentStat.ProductionCost);

    if (!bFoodComplete || !bProductionComplete)
    {
        return EBuildingType::None;
    }

    EBuildingType CompletedBuilding = m_CurrentStat.CurrentlyProducing;
    
    // 건물 추가
    AddBuilding(CompletedBuilding);

    // 생산 상태 리셋
    m_CurrentStat.CurrentlyProducing = EBuildingType::None;
    m_CurrentStat.ProductionProgress = 0;
    m_CurrentStat.ProductionCost = 0;
    m_CurrentStat.FoodProgress = 0;
    m_CurrentStat.FoodCost = 0;

    return CompletedBuilding;
}

void UCityComponent::StartUnitProduction(FName UnitName)
{
    if (UnitName == NAME_None)
    {
        return;
    }

    // 유닛 데이터에서 비용 가져오기
    if (!UnitStatusTable)
    {
        LoadUnitStatusTable();
    }

    FUnitBaseStat* UnitStat = UnitStatusTable->FindRow<FUnitBaseStat>(UnitName, TEXT("StartUnitProduction"));
    if (!UnitStat)
    {
        return;
    }

    m_CurrentStat.bIsProducingUnit = true;
    m_CurrentStat.ProducingUnitName = UnitName;
    m_CurrentStat.UnitProductionProgress = 0;
    m_CurrentStat.UnitProductionCost = UnitStat->ProductionCost;
    m_CurrentStat.UnitFoodProgress = 0;
    m_CurrentStat.UnitFoodCost = UnitStat->FoodCost;
}

void UCityComponent::ChangeUnitProduction(FName NewUnitName)
{
    if (NewUnitName == NAME_None)
    {
        return;
    }

    // 유닛 데이터에서 비용 가져오기
    if (!UnitStatusTable)
    {
        LoadUnitStatusTable();
    }

    FUnitBaseStat* UnitStat = UnitStatusTable->FindRow<FUnitBaseStat>(NewUnitName, TEXT("ChangeUnitProduction"));
    if (!UnitStat)
    {
        return;
    }

    // 건물 생산 상태 리셋 (유닛 생산으로 전환하므로)
    m_CurrentStat.CurrentlyProducing = EBuildingType::None;
    m_CurrentStat.ProductionProgress = 0;
    m_CurrentStat.ProductionCost = 0;
    m_CurrentStat.FoodProgress = 0;
    m_CurrentStat.FoodCost = 0;

    // 유닛 생산 설정 (진행도 초기화)
    m_CurrentStat.bIsProducingUnit = true;
    m_CurrentStat.ProducingUnitName = NewUnitName;
    m_CurrentStat.UnitProductionProgress = 0;
    m_CurrentStat.UnitProductionCost = UnitStat->ProductionCost;
    m_CurrentStat.UnitFoodProgress = 0;
    m_CurrentStat.UnitFoodCost = UnitStat->FoodCost;
}

void UCityComponent::UpdateUnitProductionProgress(int32 FoodAmount, int32 ProductionAmount)
{
    if (!m_CurrentStat.bIsProducingUnit)
    {
        return;
    }

    // 진행도 업데이트 (목표치를 넘지 않도록)
    if (m_CurrentStat.UnitFoodCost > 0)
    {
        m_CurrentStat.UnitFoodProgress += FoodAmount;
        m_CurrentStat.UnitFoodProgress = FMath::Min(m_CurrentStat.UnitFoodProgress, m_CurrentStat.UnitFoodCost);
    }
    
    if (m_CurrentStat.UnitProductionCost > 0)
    {
        m_CurrentStat.UnitProductionProgress += ProductionAmount;
        m_CurrentStat.UnitProductionProgress = FMath::Min(m_CurrentStat.UnitProductionProgress, m_CurrentStat.UnitProductionCost);
    }

    // 완료 조건 확인 및 완료 처리
    CompleteUnitProduction();
}

FName UCityComponent::CompleteUnitProduction()
{
    if (!m_CurrentStat.bIsProducingUnit)
    {
        return NAME_None;
    }

    // 완료 조건 확인
    bool bFoodComplete = (m_CurrentStat.UnitFoodCost <= 0 || m_CurrentStat.UnitFoodProgress >= m_CurrentStat.UnitFoodCost);
    bool bProductionComplete = (m_CurrentStat.UnitProductionCost <= 0 || m_CurrentStat.UnitProductionProgress >= m_CurrentStat.UnitProductionCost);

    if (!bFoodComplete || !bProductionComplete)
    {
        return NAME_None;
    }

    FName CompletedUnitName = m_CurrentStat.ProducingUnitName;

    // 생산 상태 리셋
    m_CurrentStat.bIsProducingUnit = false;
    m_CurrentStat.ProducingUnitName = NAME_None;
    m_CurrentStat.UnitProductionProgress = 0;
    m_CurrentStat.UnitProductionCost = 0;
    m_CurrentStat.UnitFoodProgress = 0;
    m_CurrentStat.UnitFoodCost = 0;

    return CompletedUnitName;
}

void UCityComponent::TakeDamage(int32 DamageAmount)
{
    if (m_CurrentStat.RemainingHealth <= 0)
    {
        return;
    }

    m_CurrentStat.RemainingHealth -= DamageAmount;
    m_CurrentStat.RemainingHealth = FMath::Max(0, m_CurrentStat.RemainingHealth);

    if (m_CurrentStat.RemainingHealth <= 0)
    {
        // 도시가 파괴됨
        // 필요한 처리 추가 가능
    }
}

void UCityComponent::Heal(int32 HealAmount)
{
    m_CurrentStat.RemainingHealth += HealAmount;
    m_CurrentStat.RemainingHealth = FMath::Min(m_FinalStat.MaxHealth, m_CurrentStat.RemainingHealth);
}

int32 UCityComponent::GetMaxHealth() const
{
    return m_FinalStat.MaxHealth;
}

