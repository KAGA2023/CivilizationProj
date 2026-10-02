// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"
#include "CityProductionSlotUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnProductionSlotClicked, FName, ProductionID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnProductionSlotHovered, FName, ProductionID, bool, bIsBuilding);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnProductionSlotUnhovered);

UCLASS()
class CIVILIZATION_API UCityProductionSlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UCityProductionSlotUI(const FObjectInitializer& ObjectInitializer);

	// ========== 델리게이트 ==========

	// 생산 슬롯을 클릭했을 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Production Slot")
	FOnProductionSlotClicked OnProductionSlotClicked;

	// 생산 슬롯을 호버했을 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Production Slot")
	FOnProductionSlotHovered OnProductionSlotHovered;

	// 생산 슬롯 호버가 끝났을 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Production Slot")
	FOnProductionSlotUnhovered OnProductionSlotUnhovered;

	// ========== 슬롯 데이터 ==========

	// 이 슬롯이 가리키는 건물 또는 유닛 RowName
	UPROPERTY(BlueprintReadWrite, Category = "Production Slot")
	FName ProductionID = NAME_None;

	// 건물 슬롯이면 true, 유닛 슬롯이면 false. 호버 시 정보 위젯을 고릅니다.
	UPROPERTY(BlueprintReadWrite, Category = "Production Slot")
	bool bIsBuildingSlot = false;

	// ========== 표시 설정 ==========

	// 생산 항목 아이콘 이미지를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Production Slot")
	void SetProductionImage(UTexture2D* Texture);

	// 생산 항목 이름 텍스트를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Production Slot")
	void SetProductionItemText(const FString& Text);

	// 전략 자원이 없을 때 남은 턴 수 텍스트를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Production Slot")
	void SetTurnText(int32 Turns);

	// 전략 자원이 필요할 때 턴 표시를 숨기고 자원 표시를 켭니다.
	UFUNCTION(BlueprintCallable, Category = "Production Slot")
	void SetStrategicResourceDisplay(int32 Turns, int32 RequiredAmount, UTexture2D* ResourceIconTexture);

protected:
	virtual void NativeConstruct() override;

	// ========== 버튼 핸들러 ==========

	// 생산 시작 버튼을 누르면 클릭 델리게이트를 방송합니다.
	UFUNCTION()
	void OnStartProductionBtnClicked();

	// 생산 시작 버튼을 호버하면 호버 델리게이트를 방송합니다.
	UFUNCTION()
	void OnStartProductionBtnHovered();

	// 생산 시작 버튼 호버가 끝나면 언호버 델리게이트를 방송합니다.
	UFUNCTION()
	void OnStartProductionBtnUnhovered();

	// ========== BindWidget ==========

	// 생산 항목 아이콘 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ProductionImg = nullptr;

	// 생산 항목 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ProductionItemTxt = nullptr;

	// 전략 자원이 없을 때 보여주는 남은 턴 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TurnTxt = nullptr;

	// 전략 자원이 없을 때 보여주는 시간 아이콘
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* TimeImage = nullptr;

	// 이 항목 생산을 시작하는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* StartProductionBtn = nullptr;

	// 전략 자원이 필요할 때 보여주는 남은 턴 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* ResourceTurnTxt = nullptr;

	// 전략 자원이 필요할 때 보여주는 시간 아이콘
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* ResourceTimeImage = nullptr;

	// 필요한 전략 자원 수량 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* ResourceStockTxt = nullptr;

	// 필요한 전략 자원 아이콘
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* ResourceIconImg = nullptr;
};
