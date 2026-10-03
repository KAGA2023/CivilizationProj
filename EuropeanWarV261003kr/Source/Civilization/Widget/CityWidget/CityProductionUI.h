// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "CityProductionUI.generated.h"

class ASuperPlayerState;
class UCityComponent;
class UCityProductionSlotUI;
class UCityBuildingSlotInfoUI;
class UCityUnitSlotInfoUI;

UCLASS()
class CIVILIZATION_API UCityProductionUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UCityProductionUI(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

public:
	// 로컬 플레이어의 생산 가능 건물·유닛으로 목록을 채웁니다.
	UFUNCTION(BlueprintCallable, Category = "City Production")
	void SetupProductionUI(ASuperPlayerState* PlayerState);

	// ========== BindWidget ==========

	// 생산 가능 건물 슬롯을 담는 세로 박스
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* BuildingVB = nullptr;

	// 생산 가능 유닛 슬롯을 담는 세로 박스
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* UnitVB = nullptr;

	// 건물 슬롯 호버 시 보여주는 건물 산출 정보 위젯
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UCityBuildingSlotInfoUI* BuildingSlotInfoWidget = nullptr;

	// 유닛 슬롯 호버 시 보여주는 유닛 스탯 정보 위젯
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UCityUnitSlotInfoUI* UnitSlotInfoWidget = nullptr;

private:
	// ========== 바인딩 / 핸들러 ==========

	// 생산 가능 목록 갱신 델리게이트를 연결합니다.
	void BindToAvailableProductionsUpdated();

	// 시설 변경 델리게이트를 연결합니다.
	void BindToFacilityDelegates();

	// 시설 변경 델리게이트를 해제합니다.
	void UnbindFromFacilityDelegates();

	// 소유 타일 변경 델리게이트를 연결합니다.
	void BindToOwnedTilesChanged();

	// 생산 가능 목록이 바뀌면 슬롯을 다시 만듭니다.
	UFUNCTION()
	void OnAvailableProductionsUpdated(TArray<FName> AvailableBuildings, TArray<FName> AvailableUnits);

	// 소유 타일이 바뀌면 건물·유닛 목록을 다시 그립니다.
	UFUNCTION()
	void OnOwnedTilesChanged();

	// 생산 슬롯을 클릭하면 해당 항목 생산을 시작합니다.
	UFUNCTION()
	void OnProductionSlotClicked(FName ProductionID);

	// 생산 슬롯을 호버하면 건물 또는 유닛 정보 위젯을 채웁니다.
	UFUNCTION()
	void OnProductionSlotHovered(FName ProductionID, bool bIsBuilding);

	// 생산 슬롯 호버가 끝나면 정보 위젯을 숨깁니다.
	UFUNCTION()
	void OnProductionSlotUnhovered();

	// 시설이 바뀌면 슬롯 턴 수를 다시 계산합니다.
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);

	// ========== 슬롯 ==========

	// 건물 생산 슬롯을 만들어 세로 박스에 넣습니다.
	void CreateBuildingSlots(const TArray<FName>& BuildingNames);

	// 유닛 생산 슬롯을 만들어 세로 박스에 넣습니다.
	void CreateUnitSlots(const TArray<FName>& UnitNames);

	// 기존 생산 슬롯을 모두 지웁니다.
	void ClearAllSlots();

	// 모든 슬롯의 남은 턴 텍스트를 갱신합니다.
	void UpdateAllSlotsTurnText();

	// ========== 캐시 ==========

	// 생산 UI가 가리키는 플레이어 스테이트
	UPROPERTY()
	ASuperPlayerState* CachedPlayerState = nullptr;

	// 생산 UI가 가리키는 도시 컴포넌트
	UPROPERTY()
	UCityComponent* CachedCityComponent = nullptr;
};
