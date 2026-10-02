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
    // ========== 컴포넌트 ==========

    // 체력·이동력·공격 가능 여부를 담는 스테이터스
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Status")
    UUnitStatusComponent* UnitStatusComponent;

    // 전투 계산을 담당하는 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Combat")
    UUnitCombatComponent* UnitCombatComponent;

    // 이동·전투 연출을 담당하는 컴포넌트
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Visualization")
    UUnitVisualizationComponent* UnitVisualizationComponent;

    // 스켈레톤 소켓에 붙는 무기 메시
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit Visualization")
    UStaticMeshComponent* WeaponMeshComponent;

    // 머리 위 SmallUnitUI
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Unit UI")
    UWidgetComponent* SmallUnitWidgetComponent;

    // ========== 데이터 ==========

    // 외형·애님·사운드용 데이터 테이블
    UPROPERTY()
    UDataTable* UnitDataTable = nullptr;

    // FUnitData를 찾을 때 쓰는 RowName
    UPROPERTY()
    FName UnitDataRowName = NAME_None;

    // ========== 소유 / 선택 ==========

    // 이 유닛을 소유한 문명 슬롯. 0=호스트/싱글, 1=1v1 참가자, 나머지는 AI
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Ownership")
    int32 PlayerIndex = -1;

    // 지금 선택된 유닛인지 여부
    UPROPERTY()
    bool bIsSelected = false;

protected:
    virtual void BeginPlay() override;

public:
    // ========== 초기화 ==========

    // RowName으로 외형·스테이터스·사운드를 맞춥니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Initialization")
    void InitializeUnit(const FName& RowName);

    // ========== 컴포넌트 접근 ==========

    // 스테이터스 컴포넌트를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Status")
    UUnitStatusComponent* GetUnitStatusComponent() const { return UnitStatusComponent; }

    // 전투 컴포넌트를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Combat")
    UUnitCombatComponent* GetUnitCombatComponent() const { return UnitCombatComponent; }

    // 시각화 컴포넌트를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Visualization")
    UUnitVisualizationComponent* GetUnitVisualizationComponent() const { return UnitVisualizationComponent; }

    // 무기 메시를 반환합니다. 원거리 발사 시작 위치 등에 씁니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Visualization")
    UStaticMeshComponent* GetWeaponMeshComponent() const { return WeaponMeshComponent; }

    // 머리 위 SmallUnitUI를 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit UI")
    USmallUnitUI* GetSmallUnitUI() const;

    // ========== 선택 / 외곽선 ==========

    // 선택 여부를 바꾸고 외곽선을 맞춥니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void SetUnitSelected(bool bSelected);

    // 지금 선택된 유닛인지 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    bool IsUnitSelected() const { return bIsSelected; }

    // 노란 외곽선을 켜거나 끕니다. Stencil 3입니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void SetUnitOutline(bool bShowOutline);
    
    // 빨간 외곽선을 켜거나 끕니다. Stencil 4입니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Selection")
    void SetEnemyOutline(bool bShowOutline);

    // ========== 소유 / 데이터 ==========

    // 소유 문명 슬롯을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Player Ownership")
    int32 GetPlayerIndex() const { return PlayerIndex; }

    // 소유 문명 슬롯을 설정합니다.
    UFUNCTION(BlueprintCallable, Category = "Player Ownership")
    void SetPlayerIndex(int32 InPlayerIndex) { PlayerIndex = InPlayerIndex; }

    // 유닛 데이터 RowName을 반환합니다.
    UFUNCTION(BlueprintCallable, Category = "Unit Data")
    FName GetUnitDataRowName() const { return UnitDataRowName; }

    // FUnitData를 반환합니다. C++ 전용입니다.
    const struct FUnitData* GetUnitData() const;

protected:
    // ========== 내부 설정 ==========

    // 유닛 데이터 테이블을 로드합니다.
    void LoadUnitDataTable();

    // 스켈레탈 메시와 애님 클래스를 맞춥니다.
    void SetupUnitMesh(const FUnitData& UnitData);

    // 무기 메시를 소켓에 붙입니다.
    void SetupWeaponMesh(const FUnitData& UnitData);

    // 피격·공격·사망·선택 사운드를 맞춥니다.
    void SetupUnitSounds(const FUnitData& UnitData);

    // 머리 위 UI를 맞춥니다.
    void SetupUnitUI(const FUnitData& UnitData);
};
