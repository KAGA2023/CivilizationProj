// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Components/StaticMeshComponent.h"
#include "../Status/UnitStatusComponent.h"
#include "../Status/UnitDataStruct.h"
#include "UnitCharacterBase.generated.h"

class UUnitStatusComponent;
class UUnitCombatComponent;
class UUnitVisualizationComponent;
class UDataTable;
class AUnitAIController;
class UWidgetComponent;
class USmallUnitUI;

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

    // 유닛 시각화 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Visualization")
    UUnitVisualizationComponent* UnitVisualizationComponent;

    // 무기 스태틱 메시 (스켈레톤 소켓에 부착)
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Visualization")
    UStaticMeshComponent* WeaponMeshComponent;

    // 유닛 머리 위 UI 위젯 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit UI")
    UWidgetComponent* SmallUnitWidgetComponent;

    // 데이터 테이블 참조 (시각적 데이터만)
    UPROPERTY()
    UDataTable* UnitDataTable = nullptr;

    // 유닛 데이터 RowName (FUnitData 접근용)
    UPROPERTY()
    FName UnitDataRowName = NAME_None;

    // ========== 플레이어 소유 정보 ==========
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Ownership")
    int32 PlayerIndex = -1; // 이 유닛을 소유한 플레이어 인덱스

    // ========== 선택 상태 ==========
    UPROPERTY()
    bool bIsSelected = false; // 유닛이 선택되었는지 여부

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

    // 시각화 컴포넌트 접근
    UFUNCTION(BlueprintCallable, Category = "Unit Visualization")
    UUnitVisualizationComponent* GetUnitVisualizationComponent() const { return UnitVisualizationComponent; }

    // SmallUnitUI 위젯 접근
    UFUNCTION(BlueprintCallable, Category = "Unit UI")
    USmallUnitUI* GetSmallUnitUI() const;

    // 유닛 선택 상태 설정
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void SetUnitSelected(bool bSelected);

    // 유닛 선택 상태 확인
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    bool IsUnitSelected() const { return bIsSelected; }

    // Custom Depth Stencil 제어 (외곽선 표시)
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void SetUnitOutline(bool bShowOutline); // 외곽선 표시/숨김 (노란색, Stencil 3)
    
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void SetEnemyOutline(bool bShowOutline); // 적 유닛 외곽선 표시/숨김 (빨간색, Stencil 4)

    // 플레이어 소유 정보 접근
    UFUNCTION(BlueprintCallable, Category = "Player Ownership")
    int32 GetPlayerIndex() const { return PlayerIndex; }

    UFUNCTION(BlueprintCallable, Category = "Player Ownership")
    void SetPlayerIndex(int32 InPlayerIndex) { PlayerIndex = InPlayerIndex; }

    // 유닛 데이터 RowName 접근
    UFUNCTION(BlueprintCallable, Category = "Unit Data")
    FName GetUnitDataRowName() const { return UnitDataRowName; }

    // FUnitData 접근 함수 (C++ 전용, 블루프린트에서 구조체 포인터 반환 불가)
    const struct FUnitData* GetUnitData() const;

protected:
    // 내부 함수들
    void LoadUnitDataTable();
    void SetupUnitMesh(const FUnitData& UnitData);
    void SetupWeaponMesh(const FUnitData& UnitData);
    void SetupUnitSounds(const FUnitData& UnitData);
    void SetupUnitUI(const FUnitData& UnitData);
};