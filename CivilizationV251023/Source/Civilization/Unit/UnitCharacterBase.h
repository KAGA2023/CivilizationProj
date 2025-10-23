// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Status/UnitStatusComponent.h"
#include "../Status/UnitStatusStruct.h"
#include "UnitDataStruct.h"
#include "UnitCharacterBase.generated.h"

class UUnitStatusComponent;
class UDataTable;

UCLASS()
class CIVILIZATION_API AUnitCharacterBase : public ACharacter
{
    GENERATED_BODY()

public:
    AUnitCharacterBase();

protected:
    // 유닛 상태 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Status")
    UUnitStatusComponent* UnitStatusComponent;

    // 데이터 테이블 참조들
    UPROPERTY()
    UDataTable* UnitDataTable = nullptr;

    UPROPERTY()
    UDataTable* UnitStatusTable = nullptr;

protected:
    virtual void BeginPlay() override;

public:
    // 유닛 초기화 (RowName 기반)
    UFUNCTION(BlueprintCallable, Category = "Unit Initialization")
    void InitializeUnit(const FName& RowName);

    // 상태 컴포넌트 접근
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    UUnitStatusComponent* GetUnitStatusComponent() const { return UnitStatusComponent; }

protected:
    // 내부 함수들
    void LoadDataTables();
    void SetupUnitMesh(const FUnitData& UnitData);
    void SetupUnitSounds(const FUnitData& UnitData);
    void SetupUnitUI(const FUnitData& UnitData);
};