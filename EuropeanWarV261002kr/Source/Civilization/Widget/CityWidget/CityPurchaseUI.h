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
	// 로컬 플레이어의 구매 가능 건물·유닛으로 목록을 채웁니다.
	UFUNCTION(BlueprintCallable, Category = "City Purchase")
	void SetupPurchaseUI(ASuperPlayerState* PlayerState);

	// ========== BindWidget ==========

	// 구매 가능 건물 슬롯을 담는 세로 박스
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* BuildingVB = nullptr;

	// 구매 가능 유닛 슬롯을 담는 세로 박스
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

	// 구매 가능 목록 갱신 델리게이트를 연결합니다.
	void BindToAvailableProductionsUpdated();

	// 구매 가능 목록이 바뀌면 슬롯을 다시 만듭니다.
	UFUNCTION()
	void OnAvailableProductionsUpdated(TArray<FName> AvailableBuildings, TArray<FName> AvailableUnits);

	// 구매 슬롯을 클릭하면 해당 항목을 즉시 구매합니다.
	UFUNCTION()
	void OnPurchaseSlotClicked(FName PurchaseID);

	// 구매 슬롯을 호버하면 건물 또는 유닛 정보 위젯을 채웁니다.
	UFUNCTION()
	void OnPurchaseSlotHovered(FName PurchaseID, bool bIsBuilding);

	// 구매 슬롯 호버가 끝나면 정보 위젯을 숨깁니다.
	UFUNCTION()
	void OnPurchaseSlotUnhovered();

	// ========== 슬롯 ==========

	// 건물 구매 슬롯을 만들어 세로 박스에 넣습니다.
	void CreateBuildingSlots(const TArray<FName>& BuildingNames);

	// 유닛 구매 슬롯을 만들어 세로 박스에 넣습니다.
	void CreateUnitSlots(const TArray<FName>& UnitNames);

	// 기존 구매 슬롯을 모두 지웁니다.
	void ClearAllSlots();

	// ========== 캐시 ==========

	// 구매 UI가 가리키는 플레이어 스테이트
	UPROPERTY()
	ASuperPlayerState* CachedPlayerState = nullptr;

	// 구매 UI가 가리키는 도시 컴포넌트
	UPROPERTY()
	UCityComponent* CachedCityComponent = nullptr;
};
