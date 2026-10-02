// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/Texture2D.h"
#include "CityPurchaseSlotUI.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPurchaseSlotClicked, FName, PurchaseID);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPurchaseSlotHovered, FName, PurchaseID, bool, bIsBuilding);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPurchaseSlotUnhovered);

UCLASS()
class CIVILIZATION_API UCityPurchaseSlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UCityPurchaseSlotUI(const FObjectInitializer& ObjectInitializer);

	// ========== 델리게이트 ==========

	// 구매 슬롯을 클릭했을 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Purchase Slot")
	FOnPurchaseSlotClicked OnPurchaseSlotClicked;

	// 구매 슬롯을 호버했을 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Purchase Slot")
	FOnPurchaseSlotHovered OnPurchaseSlotHovered;

	// 구매 슬롯 호버가 끝났을 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Purchase Slot")
	FOnPurchaseSlotUnhovered OnPurchaseSlotUnhovered;

	// ========== 슬롯 데이터 ==========

	// 이 슬롯이 가리키는 건물 또는 유닛 RowName
	UPROPERTY(BlueprintReadWrite, Category = "Purchase Slot")
	FName PurchaseID = NAME_None;

	// 건물 슬롯이면 true, 유닛 슬롯이면 false. 호버 시 정보 위젯을 고릅니다.
	UPROPERTY(BlueprintReadWrite, Category = "Purchase Slot")
	bool bIsBuildingSlot = false;

	// ========== 표시 설정 ==========

	// 구매 항목 아이콘 이미지를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Purchase Slot")
	void SetPurchaseImage(UTexture2D* Texture);

	// 구매 항목 이름 텍스트를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Purchase Slot")
	void SetPurchaseItemText(const FString& Text);

	// 골드만 필요할 때 골드 비용 텍스트를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Purchase Slot")
	void SetGoldCostText(int32 GoldCost);

	// 전략 자원이 필요할 때 골드 비용을 숨기고 자원 표시를 켭니다.
	UFUNCTION(BlueprintCallable, Category = "Purchase Slot")
	void SetStrategicResourceDisplay(int32 GoldCost, int32 RequiredAmount, UTexture2D* ResourceIconTexture);

protected:
	virtual void NativeConstruct() override;

	// ========== 버튼 핸들러 ==========

	// 구매 버튼을 누르면 클릭 델리게이트를 방송합니다.
	UFUNCTION()
	void OnPurchaseBtnClicked();

	// 구매 버튼을 호버하면 호버 델리게이트를 방송합니다.
	UFUNCTION()
	void OnPurchaseBtnHovered();

	// 구매 버튼 호버가 끝나면 언호버 델리게이트를 방송합니다.
	UFUNCTION()
	void OnPurchaseBtnUnhovered();

	// ========== BindWidget ==========

	// 구매 항목 아이콘 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* PurchaseImg = nullptr;

	// 구매 항목 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PurchaseItemTxt = nullptr;

	// 골드만 필요할 때 보여주는 비용 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* GoldCostTxt = nullptr;

	// 골드만 필요할 때 보여주는 시간 아이콘
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* TimeImage = nullptr;

	// 이 항목을 즉시 구매하는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PurchaseBtn = nullptr;

	// 전략 자원이 필요할 때 보여주는 골드 비용 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* ResourceGoldCostTxt = nullptr;

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
