// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "CityProductionUI.generated.h"

class ASuperPlayerState;
class UCityComponent;
class UCityProductionSlotUI;

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

private:
	// 델리게이트 바인딩
	void BindToAvailableProductionsUpdated();

	// 건설 가능 목록 업데이트 시 호출
	UFUNCTION()
	void OnAvailableProductionsUpdated(TArray<FName> AvailableBuildings, TArray<FName> AvailableUnits);

	// 슬롯 클릭 핸들러
	UFUNCTION()
	void OnProductionSlotClicked(FName ProductionID);

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

