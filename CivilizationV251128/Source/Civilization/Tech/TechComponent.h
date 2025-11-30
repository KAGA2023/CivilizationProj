// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TechStruct.h"
#include "TechComponent.generated.h"

class UDataTable;
class ASuperPlayerState;

// 연구 시작 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTechResearchStarted, FName, TechRowName);

// 연구 완료 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTechResearchCompleted, FName, TechRowName);

// 연구 진행도 변경 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTechResearchProgressChanged);

// 연구 가능 목록 업데이트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnResearchableTechsUpdated, TArray<FName>, ResearchableTechs);

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class CIVILIZATION_API UTechComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UTechComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // 초기화 함수들
    UFUNCTION(BlueprintCallable, Category = "Tech Initialization")
    void InitializeTechSystem(ASuperPlayerState* InPlayerState);

    // 데이터 접근
    UFUNCTION(BlueprintCallable, Category = "Tech Data")
    FResearchCurrentStat GetCurrentStat() const { return m_CurrentStat; }

    UFUNCTION(BlueprintCallable, Category = "Tech Data")
    TArray<FName> GetResearchedTechs() const { return ResearchedTechs; }

    UFUNCTION(BlueprintCallable, Category = "Tech Data")
    TArray<FName> GetResearchableTechs() const { return ResearchableTechs; }

    // 기술 데이터 조회
    UFUNCTION(BlueprintCallable, Category = "Tech Management")
    FTechData GetTechDataFromTable(FName RowName) const;

    // 기술 연구 가능 여부 확인
    UFUNCTION(BlueprintCallable, Category = "Tech Management")
    bool CanResearchTech(FName TechRowName) const;

    // 기술 연구 완료 여부 확인
    UFUNCTION(BlueprintCallable, Category = "Tech Management")
    bool IsTechResearched(FName TechRowName) const;

    // 연구 시스템 - 기술 연구 시작
    UFUNCTION(BlueprintCallable, Category = "Tech Research")
    void StartTechResearch(FName TechRowName); // 기술 연구 시작/변경 (진행도 초기화)

    UFUNCTION(BlueprintCallable, Category = "Tech Research")
    void UpdateTechResearchProgress(int32 ScienceAmount); // 과학량으로 연구 진행도 업데이트

    UFUNCTION(BlueprintCallable, Category = "Tech Research")
    FName CompleteTechResearch(); // 연구 완료 처리

    // 연구 가능 목록 조회
    UFUNCTION(BlueprintCallable, Category = "Researchable Techs")
    TArray<FName> GetResearchableTechsList() const { return ResearchableTechs; }

    // 연구 가능 목록 업데이트
    UFUNCTION(BlueprintCallable, Category = "Researchable Techs")
    void UpdateResearchableTechs();

    // 연구 시작 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Tech Research")
    FOnTechResearchStarted OnTechResearchStarted;

    // 연구 완료 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Tech Research")
    FOnTechResearchCompleted OnTechResearchCompleted;

    // 연구 진행도 변경 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Tech Research")
    FOnTechResearchProgressChanged OnTechResearchProgressChanged;

    // 연구 가능 목록 업데이트 델리게이트
    UPROPERTY(BlueprintAssignable, Category = "Researchable Techs")
    FOnResearchableTechsUpdated OnResearchableTechsUpdated;

protected:
    // 현재 연구 상태
    UPROPERTY(BlueprintReadOnly, Category = "Tech Data")
    FResearchCurrentStat m_CurrentStat;

    // 기술 데이터 테이블
    UPROPERTY()
    UDataTable* TechDataTable = nullptr;

    // 연구 완료된 기술 목록
    UPROPERTY(BlueprintReadOnly, Category = "Tech Data")
    TArray<FName> ResearchedTechs;

    // 연구 가능한 기술 목록 (선행 기술 조건 만족)
    UPROPERTY(BlueprintReadOnly, Category = "Researchable Techs")
    TArray<FName> ResearchableTechs;

    // SuperPlayerState 참조
    UPROPERTY()
    TObjectPtr<ASuperPlayerState> PlayerState = nullptr;

    // 내부 함수들
    void LoadTechDataTable();
    void NotifyTechResearched(FName TechRowName); // 연구 완료 시 다른 시스템에 알림
};

