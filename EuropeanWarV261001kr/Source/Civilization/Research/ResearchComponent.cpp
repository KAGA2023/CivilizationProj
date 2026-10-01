// Fill out your copyright notice in the Description Settings.

#include "ResearchComponent.h"
#include "Engine/DataTable.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "../SuperPlayerState.h"
#include "../City/CityComponent.h"

UResearchComponent::UResearchComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UResearchComponent::BeginPlay()
{
    Super::BeginPlay();

    // 데이터 테이블 로드
    LoadTechDataTable();
}

void UResearchComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UResearchComponent::InitFromResearchData(const FResearchData& InResearchData)
{
    m_ResearchData = InResearchData;

    // 데이터 테이블 로드 (BeginPlay 전에 호출될 수 있으므로)
    if (!TechDataTable)
    {
        LoadTechDataTable();
    }

    // 연구 상태 초기화
    m_CurrentStat.DevelopingName = NAME_None;
    m_CurrentStat.DevelopingProgress = 0;
    m_CurrentStat.DevelopingCost = 0;

    // 연구 가능 목록 갱신
    UpdateResearchableTechs();
}


void UResearchComponent::LoadTechDataTable()
{
    // SoftObjectPath를 사용한 로딩
    FSoftObjectPath TechDataTablePath(TEXT("/Game/Civilization/Data/DT_TechData.DT_TechData"));
    TechDataTable = Cast<UDataTable>(TechDataTablePath.TryLoad());
}

FTechData UResearchComponent::GetTechDataFromTable(FName RowName) const
{
    if (!TechDataTable)
    {
        return FTechData();
    }

    // RowName으로 직접 데이터 로딩
    FTechData* TechData = TechDataTable->FindRow<FTechData>(RowName, TEXT("GetTechDataFromTable"));
    if (TechData)
    {
        return *TechData;
    }

    return FTechData();
}

bool UResearchComponent::CanResearchTech(FName TechRowName) const
{
    if (TechRowName == NAME_None)
    {
        return false;
    }

    // 이미 연구 완료했는지 체크
    if (IsTechResearched(TechRowName))
    {
        return false;
    }

    // 데이터 테이블에서 기술 데이터 조회
    FTechData TechData = GetTechDataFromTable(TechRowName);
    if (TechData.ScienceCost == 0 && TechData.TechName.IsEmpty())
    {
        return false; // 기술이 존재하지 않음
    }

    // 선행 기술 체크
    for (const FName& RequiredTech : TechData.RequiredTechs)
    {
        if (!IsTechResearched(RequiredTech))
        {
            return false; // 선행 기술이 연구되지 않음
        }
    }

    return true;
}

bool UResearchComponent::IsTechResearched(FName TechRowName) const
{
    if (TechRowName == NAME_None)
    {
        return false;
    }

    return m_ResearchData.ResearchedTechs.Contains(TechRowName);
}

bool UResearchComponent::IsFacilityUnlocked(FName FacilityRowName) const
{
    if (FacilityRowName == NAME_None || !TechDataTable)
    {
        return false;
    }

    // 연구 완료된 기술 목록 가져오기
    TArray<FName> ResearchedTechs = GetResearchedTechs();
    
    // 각 연구 완료된 기술의 UnlockFacilities 확인
    for (const FName& TechRowName : ResearchedTechs)
    {
        FTechData TechData = GetTechDataFromTable(TechRowName);
        if (TechData.UnlockFacilities.Contains(FacilityRowName))
        {
            return true; // 해제됨
        }
    }
    
    return false; // 해제되지 않음
}

bool UResearchComponent::IsBuildingUnlocked(FName BuildingRowName) const
{
    if (BuildingRowName == NAME_None || !TechDataTable)
    {
        return false;
    }

    // 연구 완료된 기술 목록 가져오기
    TArray<FName> ResearchedTechs = GetResearchedTechs();
    
    // 각 연구 완료된 기술의 UnlockBuildings 확인
    for (const FName& TechRowName : ResearchedTechs)
    {
        FTechData TechData = GetTechDataFromTable(TechRowName);
        if (TechData.UnlockBuildings.Contains(BuildingRowName))
        {
            return true; // 해제됨
        }
    }
    
    return false; // 해제되지 않음
}

bool UResearchComponent::IsUnitUnlocked(FName UnitRowName) const
{
    if (UnitRowName == NAME_None || !TechDataTable)
    {
        return false;
    }

    // 연구 완료된 기술 목록 가져오기
    TArray<FName> ResearchedTechs = GetResearchedTechs();
    
    // 각 연구 완료된 기술의 UnlockUnits 확인
    for (const FName& TechRowName : ResearchedTechs)
    {
        FTechData TechData = GetTechDataFromTable(TechRowName);
        if (TechData.UnlockUnits.Contains(UnitRowName))
        {
            return true; // 해제됨
        }
    }
    
    return false; // 해제되지 않음
}

void UResearchComponent::StartTechResearch(FName TechRowName)
{
    if (TechRowName == NAME_None)
    {
        return;
    }

    // 이미 같은 기술이 연구 중이면 무시
    if (m_CurrentStat.DevelopingName == TechRowName)
    {
        return;
    }

    // 연구 가능 여부 체크
    if (!CanResearchTech(TechRowName))
    {
        return;
    }

    // 데이터 테이블에서 기술 데이터 조회
    FTechData TechData = GetTechDataFromTable(TechRowName);
    if (TechData.ScienceCost == 0 && TechData.TechName.IsEmpty())
    {
        return;
    }

    // 기술 연구 시작/변경 (진행도 초기화)
    m_CurrentStat.DevelopingName = TechRowName;
    m_CurrentStat.DevelopingProgress = 0;
    m_CurrentStat.DevelopingCost = TechData.ScienceCost;

    // 연구 시작 델리게이트 브로드캐스트
    OnTechResearchStarted.Broadcast(TechRowName);
}

void UResearchComponent::UpdateTechResearchProgress(int32 ScienceAmount)
{
    // 연구 중인 기술이 없으면 종료
    if (m_CurrentStat.DevelopingName == NAME_None)
    {
        return;
    }

    // 연구 진행도 업데이트
    if (m_CurrentStat.DevelopingCost > 0)
    {
        m_CurrentStat.DevelopingProgress += ScienceAmount;
        m_CurrentStat.DevelopingProgress = FMath::Min(m_CurrentStat.DevelopingProgress, m_CurrentStat.DevelopingCost);
        
        // 진행도 변경 델리게이트 브로드캐스트
        OnTechResearchProgressChanged.Broadcast();
    }

    // 완료 조건 확인 및 완료 처리
    CompleteTechResearch();
}

void UResearchComponent::SetResearchProgress(int32 Progress)
{
    // 연구 중인 기술이 없으면 종료
    if (m_CurrentStat.DevelopingName == NAME_None)
    {
        return;
    }

    // 연구 진행도 직접 설정 (세이브/로드용)
    if (m_CurrentStat.DevelopingCost > 0)
    {
        m_CurrentStat.DevelopingProgress = FMath::Clamp(Progress, 0, m_CurrentStat.DevelopingCost);
        
        // 진행도 변경 델리게이트 브로드캐스트
        OnTechResearchProgressChanged.Broadcast();
    }
}

FName UResearchComponent::CompleteTechResearch()
{
    // 연구 중인 기술이 없으면 종료
    if (m_CurrentStat.DevelopingName == NAME_None)
    {
        return NAME_None;
    }

    // 완료 조건 확인
    bool bResearchComplete = (m_CurrentStat.DevelopingCost <= 0 || 
                              m_CurrentStat.DevelopingProgress >= m_CurrentStat.DevelopingCost);

    if (!bResearchComplete)
    {
        return NAME_None;
    }

    FName CompletedTechRowName = m_CurrentStat.DevelopingName;
    
    // 연구 상태 리셋
    m_CurrentStat.DevelopingName = NAME_None;
    m_CurrentStat.DevelopingProgress = 0;
    m_CurrentStat.DevelopingCost = 0;

    // 연구 완료된 기술 목록에 추가
    if (!m_ResearchData.ResearchedTechs.Contains(CompletedTechRowName))
    {
        m_ResearchData.ResearchedTechs.Add(CompletedTechRowName);
    }

    // 연구 가능 목록 갱신 (새로운 기술들이 해제될 수 있음)
    UpdateResearchableTechs();

    // 다른 시스템에 알림 (SuperPlayerState, CityComponent)
    NotifyTechResearched(CompletedTechRowName);

    // 연구 완료 델리게이트 브로드캐스트
    OnTechResearchCompleted.Broadcast(CompletedTechRowName);

    // 플레이어 0(본인)의 연구 완료 시 사운드 재생
    if (ASuperPlayerState* OwnerPS = Cast<ASuperPlayerState>(GetOwner()))
    {
        if (OwnerPS->PlayerIndex == 0)
        {
            if (USoundBase* ResearchSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Civilization/Sound/Research.Research")))
            {
                UGameplayStatics::PlaySound2D(this, ResearchSound);
            }
        }
    }

    return CompletedTechRowName;
}

void UResearchComponent::StopCurrentResearch()
{
    // 연구 상태 초기화
    m_CurrentStat.DevelopingName = NAME_None;
    m_CurrentStat.DevelopingProgress = 0;
    m_CurrentStat.DevelopingCost = 0;
    
    // 진행도 변경 델리게이트 브로드캐스트
    OnTechResearchProgressChanged.Broadcast();
}

void UResearchComponent::UpdateResearchableTechs()
{
    // 연구 가능한 기술 목록 초기화
    ResearchableTechs.Empty();

    // 데이터 테이블이 없으면 종료
    if (!TechDataTable)
    {
        return;
    }

    // 기술 데이터 테이블의 모든 행 순회
    TArray<FName> TechRowNames = TechDataTable->GetRowNames();

    for (const FName& RowName : TechRowNames)
    {
        // 연구 가능 여부 체크 (선행 기술 조건 포함)
        if (CanResearchTech(RowName))
        {
            ResearchableTechs.Add(RowName);
        }
    }

    // 델리게이트 브로드캐스트 (UI 업데이트용)
    OnResearchableTechsUpdated.Broadcast(ResearchableTechs);
}

void UResearchComponent::NotifyTechResearched(FName TechRowName)
{
    // GetOwner()를 통해 PlayerState 가져오기
    ASuperPlayerState* PlayerState = Cast<ASuperPlayerState>(GetOwner());
    if (!PlayerState)
    {
        return;
    }

    // 기술 데이터 조회
    FTechData TechData = GetTechDataFromTable(TechRowName);
    if (TechData.ScienceCost == 0 && TechData.TechName.IsEmpty())
    {
        return;
    }

    // SuperPlayerState의 AvailableFacilities 업데이트
    PlayerState->UpdateAvailableFacilities();

    // CityComponent의 AvailableBuildings, AvailableUnits 업데이트
    if (UCityComponent* CityComp = PlayerState->GetCityComponent())
    {
        CityComp->UpdateAvailableProductions();
    }
}

