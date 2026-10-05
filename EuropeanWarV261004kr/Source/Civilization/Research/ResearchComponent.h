// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Research.h"
#include "ResearchComponent.generated.h"

class UDataTable;

// 연구 가능 목록이 갱신될 때 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResearchableTechsUpdated, TArray<FName>, ResearchableTechs);

// 연구를 시작할 때 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTechResearchStarted, FName, TechID);

// 연구가 완료될 때 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTechResearchCompleted, FName, TechID);

// 연구 진행도가 바뀔 때 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTechResearchProgressChanged);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CIVILIZATION_API UResearchComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UResearchComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // ========== 초기화 ==========

    // 세이브 데이터로 완료 기술을 넣고 연구 가능 목록을 갱신합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Initialization")
    void InitFromResearchData(const FResearchData& InResearchData);

    // ========== 데이터 조회 ==========

    // 연구 완료 목록을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Data")
    FResearchData GetResearchData() const { return m_ResearchData; }

    // 현재 연구 진행 상태를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Data")
    FResearchCurrentStat GetCurrentStat() const { return m_CurrentStat; }

    // 연구 완료된 기술 RowName 배열을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Data")
    TArray<FName> GetResearchedTechs() const { return m_ResearchData.ResearchedTechs; }

    // 지금 연구 가능한 기술 RowName 배열을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Data")
    TArray<FName> GetResearchableTechs() const { return ResearchableTechs; }

    // ========== 기술 판정 ==========

    // 데이터 테이블에서 기술 한 줄을 읽습니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Management")
    FTechData GetTechDataFromTable(FName RowName) const;

    // 해당 기술을 지금 연구할 수 있는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Management")
    bool CanResearchTech(FName TechRowName) const;

    // 해당 기술을 이미 연구했는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Management")
    bool IsTechResearched(FName TechRowName) const;

    // 시설이 기술로 해제됐는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Management")
    bool IsFacilityUnlocked(FName FacilityRowName) const;

    // 건물이 기술로 해제됐는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Management")
    bool IsBuildingUnlocked(FName BuildingRowName) const;

    // 유닛이 기술로 해제됐는지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Management")
    bool IsUnitUnlocked(FName UnitRowName) const;

    // ========== 연구 진행 ==========

    // 기술 연구를 시작하거나 바꿉니다. 진행도는 초기화합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Research")
    void StartTechResearch(FName TechRowName);

    // 과학량만큼 연구 진행도를 올립니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Research")
    void UpdateTechResearchProgress(int32 ScienceAmount);

    // 연구 진행도를 직접 넣습니다. 세이브/로드용입니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Research")
    void SetResearchProgress(int32 Progress);

    // 현재 연구를 완료 처리하고 기술 RowName을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Research")
    FName CompleteTechResearch();

    // 현재 연구를 중단하고 진행도를 초기화합니다.
    UFUNCTION(BlueprintCallable, Category = "Tech Research")
    void StopCurrentResearch();

    // ========== 연구 가능 목록 ==========

    // 연구 가능 목록을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Researchable Techs")
    TArray<FName> GetResearchableTechsList() const { return ResearchableTechs; }

    // 선행 조건을 다시 계산해 연구 가능 목록을 갱신합니다.
    UFUNCTION(BlueprintCallable, Category = "Researchable Techs")
    void UpdateResearchableTechs();

    // ========== 이벤트 ==========

    // 연구를 시작할 때 브로드캐스트합니다.
    UPROPERTY(BlueprintAssignable, Category = "Tech Research")
    FOnTechResearchStarted OnTechResearchStarted;

    // 연구가 완료될 때 브로드캐스트합니다.
    UPROPERTY(BlueprintAssignable, Category = "Tech Research")
    FOnTechResearchCompleted OnTechResearchCompleted;

    // 연구 진행도가 바뀔 때 브로드캐스트합니다.
    UPROPERTY(BlueprintAssignable, Category = "Tech Research")
    FOnTechResearchProgressChanged OnTechResearchProgressChanged;

    // 연구 가능 목록이 갱신될 때 브로드캐스트합니다.
    UPROPERTY(BlueprintAssignable, Category = "Researchable Techs")
    FOnResearchableTechsUpdated OnResearchableTechsUpdated;

protected:
    // ========== 상태 ==========

    // 연구 완료된 기술 목록
    UPROPERTY(BlueprintReadOnly, Category = "Tech Data")
    FResearchData m_ResearchData;

    // 지금 개발 중인 기술과 진행도
    UPROPERTY(BlueprintReadOnly, Category = "Tech Data")
    FResearchCurrentStat m_CurrentStat;

    // 기술 데이터 테이블
    UPROPERTY()
    UDataTable* TechDataTable = nullptr;

    // 선행 조건을 만족해 연구 가능한 기술 목록
    UPROPERTY(BlueprintReadOnly, Category = "Researchable Techs")
    TArray<FName> ResearchableTechs;

    // ========== 내부 ==========

    // 기술 데이터 테이블을 로드합니다.
    void LoadTechDataTable();

    // 연구 완료를 다른 시스템에 알립니다.
    void NotifyTechResearched(FName TechRowName);
};
