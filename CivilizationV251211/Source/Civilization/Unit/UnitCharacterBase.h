// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Status/UnitStatusComponent.h"
#include "../Status/UnitDataStruct.h"
#include "UnitCharacterBase.generated.h"

class UUnitStatusComponent;
class UUnitCombatComponent;
class UDataTable;
class AUnitAIController;

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

    // 유닛 전투 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Combat")
    UUnitCombatComponent* UnitCombatComponent;

    // 데이터 테이블 참조 (시각적 데이터만)
    UPROPERTY()
    UDataTable* UnitDataTable = nullptr;

    // 유닛 데이터 RowName (FUnitData 접근용)
    UPROPERTY()
    FName UnitDataRowName = NAME_None;

    // ========== 플레이어 소유 정보 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Ownership")
    int32 PlayerIndex = -1; // 이 유닛을 소유한 플레이어 인덱스

protected:
    virtual void BeginPlay() override;

public:
    // 유닛 초기화 (RowName 기반)
    UFUNCTION(BlueprintCallable, Category = "Unit Initialization")
    void InitializeUnit(const FName& RowName);

    // 상태 컴포넌트 접근
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    UUnitStatusComponent* GetUnitStatusComponent() const { return UnitStatusComponent; }

    // 전투 컴포넌트 접근
    UFUNCTION(BlueprintCallable, Category = "Unit Combat")
    UUnitCombatComponent* GetUnitCombatComponent() const { return UnitCombatComponent; }

    // 플레이어 소유 정보 접근
    UFUNCTION(BlueprintCallable, Category = "Player Ownership")
    int32 GetPlayerIndex() const { return PlayerIndex; }

    UFUNCTION(BlueprintCallable, Category = "Player Ownership")
    void SetPlayerIndex(int32 InPlayerIndex) { PlayerIndex = InPlayerIndex; }

    // FUnitData 접근 함수 (C++ 전용, 블루프린트에서 구조체 포인터 반환 불가)
    const struct FUnitData* GetUnitData() const;

protected:
    // 내부 함수들
    void LoadUnitDataTable();
    void SetupUnitMesh(const FUnitData& UnitData);
    void SetupUnitSounds(const FUnitData& UnitData);
    void SetupUnitUI(const FUnitData& UnitData);
};