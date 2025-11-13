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
    m_CurrentStat.ProductionType = EProductionType::None;
    m_CurrentStat.ProductionName = NAME_None;
    m_CurrentStat.ProductionProgress = 0;
    m_CurrentStat.ProductionCost = 0;
    m_CurrentStat.FoodProgress = 0;
    m_CurrentStat.FoodCost = 0;

    // 생산량 재계산
    RecalculateYields();

    // 최종 체력으로 현재 체력 설정
    m_CurrentStat.RemainingHealth = m_FinalStat.MaxHealth;

    // 건설 가능 목록 갱신
    UpdateAvailableProductions();
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
    for (const FName& BuildingRowName : m_CityData.BuiltBuildings)
    {
        FBuildingData BuildingData = GetBuildingDataFromTable(BuildingRowName);
        m_FinalStat.FoodYield += BuildingData.FoodYield;
        m_FinalStat.ProductionYield += BuildingData.ProductionYield;
        m_FinalStat.GoldYield += BuildingData.GoldYield;
        m_FinalStat.ScienceYield += BuildingData.ScienceYield;
        m_FinalStat.FaithYield += BuildingData.FaithYield;
        m_FinalStat.MaxHealth += BuildingData.MaxHealth;
    }
}

void UCityComponent::AddBuilding(FName BuildingRowName)
{
    if (BuildingRowName == NAME_None)
    {
        return;
    }

    // 데이터 테이블에서 건물 데이터 조회하여 유효성 확인
    FBuildingData BuildingData = GetBuildingDataFromTable(BuildingRowName);
    if (BuildingData.BuildingType == EBuildingType::None)
    {
        return;
    }

    // 이미 건설된 건물이면 추가하지 않음 (RowName으로 중복 체크)
    if (HasBuilding(BuildingRowName))
    {
        return;
    }

    m_CityData.BuiltBuildings.Add(BuildingRowName);

    // 생산량 재계산
    RecalculateYields();

    // 건설 가능 목록 갱신
    UpdateAvailableProductions();

    // 건물 추가 완료 델리게이트 브로드캐스트 (구매/생산 완료 모두 동일하게 처리)
    OnProductionCompleted.Broadcast(BuildingRowName);
}

void UCityComponent::RemoveBuilding(FName BuildingRowName)
{
    if (BuildingRowName == NAME_None)
    {
        return;
    }

    // 첫 번째로 발견된 해당 RowName의 건물 제거
    m_CityData.BuiltBuildings.RemoveSingle(BuildingRowName);

    // 생산량 재계산
    RecalculateYields();
}

bool UCityComponent::HasBuilding(FName BuildingRowName) const
{
    if (BuildingRowName == NAME_None)
    {
        return false;
    }

    return m_CityData.BuiltBuildings.Contains(BuildingRowName);
}

FBuildingData UCityComponent::GetBuildingDataFromTable(FName RowName) const
{
    if (!BuildingDataTable)
    {
        return FBuildingData();
    }

    // RowName으로 직접 데이터 로딩
    FBuildingData* BuildingData = BuildingDataTable->FindRow<FBuildingData>(RowName, TEXT("GetBuildingDataFromTable"));
    if (BuildingData)
    {
        return *BuildingData;
    }

    return FBuildingData();
}

FUnitBaseStat UCityComponent::GetUnitDataFromTable(FName RowName) const
{
    if (!UnitStatusTable)
    {
        return FUnitBaseStat();
    }

    // RowName으로 직접 데이터 로딩
    FUnitBaseStat* UnitStat = UnitStatusTable->FindRow<FUnitBaseStat>(RowName, TEXT("GetUnitDataFromTable"));
    if (UnitStat)
    {
        return *UnitStat;
    }

    return FUnitBaseStat();
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

void UCityComponent::StartBuildingProduction(FName BuildingRowName)
{
    if (BuildingRowName == NAME_None)
    {
        return;
    }

    // 이미 같은 건물이 생산 중이면 무시
    if (m_CurrentStat.ProductionType == EProductionType::Building && 
        m_CurrentStat.ProductionName == BuildingRowName)
    {
        return;
    }

    // 데이터 테이블에서 건물 데이터 조회
    FBuildingData BuildingData = GetBuildingDataFromTable(BuildingRowName);
    if (BuildingData.BuildingType == EBuildingType::None)
    {
        return;
    }

    // 이미 건설된 건물이면 생산 불가 (RowName으로 중복 체크)
    if (HasBuilding(BuildingRowName))
    {
        return;
    }

    // 건물 생산 시작/변경 (진행도 초기화)
    // 건물은 생산력만 사용 (식량 사용 안 함)
    m_CurrentStat.ProductionType = EProductionType::Building;
    m_CurrentStat.ProductionName = BuildingRowName;
    m_CurrentStat.ProductionProgress = 0;
    m_CurrentStat.ProductionCost = BuildingData.ProductionCost;
    m_CurrentStat.FoodProgress = 0;
    m_CurrentStat.FoodCost = 0; // 건물은 식량 비용 없음

    // 생산 시작 델리게이트 브로드캐스트
    OnProductionStarted.Broadcast(BuildingRowName);
}

void UCityComponent::UpdateBuildingProductionProgress(int32 ProductionAmount)
{
    if (m_CurrentStat.ProductionType != EProductionType::Building)
    {
        return;
    }

    // 건물은 생산력만 사용 (식량 사용 안 함)
    if (m_CurrentStat.ProductionCost > 0)
    {
        m_CurrentStat.ProductionProgress += ProductionAmount;
        m_CurrentStat.ProductionProgress = FMath::Min(m_CurrentStat.ProductionProgress, m_CurrentStat.ProductionCost);
        
        // 진행도 변경 델리게이트 브로드캐스트
        OnProductionProgressChanged.Broadcast();
    }

    // 완료 조건 확인 및 완료 처리
    CompleteBuildingProduction();
}

FName UCityComponent::CompleteBuildingProduction()
{
    if (m_CurrentStat.ProductionType != EProductionType::Building)
    {
        return NAME_None;
    }

    // 완료 조건 확인 (건물은 생산력만 사용)
    bool bProductionComplete = (m_CurrentStat.ProductionCost <= 0 || m_CurrentStat.ProductionProgress >= m_CurrentStat.ProductionCost);

    if (!bProductionComplete)
    {
        return NAME_None;
    }

    FName CompletedBuildingRowName = m_CurrentStat.ProductionName;
    
    // 생산 상태 리셋
    m_CurrentStat.ProductionType = EProductionType::None;
    m_CurrentStat.ProductionName = NAME_None;
    m_CurrentStat.ProductionProgress = 0;
    m_CurrentStat.ProductionCost = 0;
    m_CurrentStat.FoodProgress = 0;
    m_CurrentStat.FoodCost = 0;

    // 건물 추가 (AddBuilding 내부에서 OnProductionCompleted 델리게이트 브로드캐스트)
    AddBuilding(CompletedBuildingRowName);

    return CompletedBuildingRowName;
}

void UCityComponent::StartUnitProduction(FName UnitName)
{
    if (UnitName == NAME_None)
    {
        return;
    }

    // 이미 같은 유닛이 생산 중이면 무시
    if (m_CurrentStat.ProductionType == EProductionType::Unit && 
        m_CurrentStat.ProductionName == UnitName)
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

    // 유닛 생산 시작/변경 (진행도 초기화)
    // 유닛은 식량만 사용 (생산력 사용 안 함)
    m_CurrentStat.ProductionType = EProductionType::Unit;
    m_CurrentStat.ProductionName = UnitName;
    m_CurrentStat.ProductionProgress = 0;
    m_CurrentStat.ProductionCost = 0; // 유닛은 생산력 비용 없음
    m_CurrentStat.FoodProgress = 0;
    m_CurrentStat.FoodCost = UnitStat->FoodCost;

    // 생산 시작 델리게이트 브로드캐스트
    OnProductionStarted.Broadcast(UnitName);
}

FName UCityComponent::UpdateUnitProductionProgress(int32 FoodAmount)
{
    if (m_CurrentStat.ProductionType != EProductionType::Unit)
    {
        return NAME_None;
    }

    // 유닛은 식량만 사용 (생산력 사용 안 함)
    if (m_CurrentStat.FoodCost > 0)
    {
        m_CurrentStat.FoodProgress += FoodAmount;
        m_CurrentStat.FoodProgress = FMath::Min(m_CurrentStat.FoodProgress, m_CurrentStat.FoodCost);
        
        // 진행도 변경 델리게이트 브로드캐스트
        OnProductionProgressChanged.Broadcast();
    }

    // 완료 조건 확인 및 완료 처리
    return CompleteUnitProduction();
}

FName UCityComponent::CompleteUnitProduction()
{
    if (m_CurrentStat.ProductionType != EProductionType::Unit)
    {
        return NAME_None;
    }

    // 완료 조건 확인 (유닛은 식량만 확인)
    bool bFoodComplete = (m_CurrentStat.FoodCost <= 0 || m_CurrentStat.FoodProgress >= m_CurrentStat.FoodCost);

    if (!bFoodComplete)
    {
        return NAME_None;
    }

    FName CompletedUnitName = m_CurrentStat.ProductionName;

    // 생산 상태 리셋
    m_CurrentStat.ProductionType = EProductionType::None;
    m_CurrentStat.ProductionName = NAME_None;
    m_CurrentStat.ProductionProgress = 0;
    m_CurrentStat.ProductionCost = 0;
    m_CurrentStat.FoodProgress = 0;
    m_CurrentStat.FoodCost = 0;

    // 생산 완료 델리게이트 브로드캐스트
    OnProductionCompleted.Broadcast(CompletedUnitName);

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

void UCityComponent::UpdateAvailableProductions()
{
    // 건설 가능한 건물 목록 초기화
    AvailableBuildings.Empty();
    AvailableUnits.Empty();

    // 데이터 테이블이 없으면 종료
    if (!BuildingDataTable || !UnitStatusTable)
    {
        return;
    }

    // 건물 데이터 테이블의 모든 행 순회
    TArray<FName> BuildingRowNames = BuildingDataTable->GetRowNames();

    for (const FName& RowName : BuildingRowNames)
    {
        // 이미 건설된 건물이면 제외
        if (HasBuilding(RowName))
        {
            continue;
        }

        // 건설 가능한 건물 목록에 추가
        AvailableBuildings.Add(RowName);
    }

    // 유닛 데이터 테이블의 모든 행 순회
    TArray<FName> UnitRowNames = UnitStatusTable->GetRowNames();

    for (const FName& RowName : UnitRowNames)
    {
        // 생산 가능한 유닛 목록에 추가 (현재는 모든 유닛 추가)
        AvailableUnits.Add(RowName);
    }

    // 델리게이트 브로드캐스트 (UI 업데이트용)
    OnAvailableProductionsUpdated.Broadcast(AvailableBuildings, AvailableUnits);
}

