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
	// 설정 함수
	UFUNCTION(BlueprintCallable, Category = "City Production")
	void SetupProductionUI(ASuperPlayerState* PlayerState);

	// 건물 VerticalBox
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* BuildingVB = nullptr;

	// 유닛 VerticalBox
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* UnitVB = nullptr;

	// 건물 슬롯 호버 시 표시할 건물 생산량 정보 위젯 (이름 일치 시 자동 바인드)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UCityBuildingSlotInfoUI* BuildingSlotInfoWidget = nullptr;

	// 유닛 슬롯 호버 시 표시할 유닛 베이스 스탯 정보 위젯 (이름 일치 시 자동 바인드)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UCityUnitSlotInfoUI* UnitSlotInfoWidget = nullptr;

private:
	// 델리게이트 바인딩
	void BindToAvailableProductionsUpdated();
	void BindToFacilityDelegates();
	void UnbindFromFacilityDelegates();
	void BindToOwnedTilesChanged();

	// 건설 가능 목록 업데이트 시 호출
	UFUNCTION()
	void OnAvailableProductionsUpdated(TArray<FName> AvailableBuildings, TArray<FName> AvailableUnits);

	// 소유 타일 변경 시 호출 (타일 구매 등 → BuildingVB/UnitVB 갱신)
	UFUNCTION()
	void OnOwnedTilesChanged();

	// 슬롯 클릭 핸들러
	UFUNCTION()
	void OnProductionSlotClicked(FName ProductionID);

	UFUNCTION()
	void OnProductionSlotHovered(FName ProductionID, bool bIsBuilding);

	UFUNCTION()
	void OnProductionSlotUnhovered();

	// 시설 변경 델리게이트 핸들러
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);

	// 슬롯 생성 및 추가
	void CreateBuildingSlots(const TArray<FName>& BuildingNames);
	void CreateUnitSlots(const TArray<FName>& UnitNames);

	// 기존 슬롯 제거
	void ClearAllSlots();

	// 모든 슬롯의 TurnTxt 갱신
	void UpdateAllSlotsTurnText();

	// 플레이어 상태 참조
	UPROPERTY()
	ASuperPlayerState* CachedPlayerState = nullptr;

	// 도시 컴포넌트 참조
	UPROPERTY()
	UCityComponent* CachedCityComponent = nullptr;
};

