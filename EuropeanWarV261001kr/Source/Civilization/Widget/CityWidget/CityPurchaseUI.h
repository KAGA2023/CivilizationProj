// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "CityPurchaseUI.generated.h"

class ASuperPlayerState;
class UCityComponent;
class UCityPurchaseSlotUI;
class UCityBuildingSlotInfoUI;
class UCityUnitSlotInfoUI;

UCLASS()
class CIVILIZATION_API UCityPurchaseUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UCityPurchaseUI(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

public:
	// 설정 함수
	UFUNCTION(BlueprintCallable, Category = "City Purchase")
	void SetupPurchaseUI(ASuperPlayerState* PlayerState);

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

	// 건설 가능 목록 업데이트 시 호출
	UFUNCTION()
	void OnAvailableProductionsUpdated(TArray<FName> AvailableBuildings, TArray<FName> AvailableUnits);

	// 슬롯 클릭 핸들러
	UFUNCTION()
	void OnPurchaseSlotClicked(FName PurchaseID);

	UFUNCTION()
	void OnPurchaseSlotHovered(FName PurchaseID, bool bIsBuilding);

	UFUNCTION()
	void OnPurchaseSlotUnhovered();

	// 슬롯 생성 및 추가
	void CreateBuildingSlots(const TArray<FName>& BuildingNames);
	void CreateUnitSlots(const TArray<FName>& UnitNames);

	// 기존 슬롯 제거
	void ClearAllSlots();

	// 플레이어 상태 참조
	UPROPERTY()
	ASuperPlayerState* CachedPlayerState = nullptr;

	// 도시 컴포넌트 참조
	UPROPERTY()
	UCityComponent* CachedCityComponent = nullptr;
};

