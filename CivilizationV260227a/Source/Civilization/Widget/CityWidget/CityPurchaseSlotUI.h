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

UCLASS()
class CIVILIZATION_API UCityPurchaseSlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UCityPurchaseSlotUI(const FObjectInitializer& ObjectInitializer);

	// 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Purchase Slot")
	FOnPurchaseSlotClicked OnPurchaseSlotClicked;

	// 구매 ID (건물 또는 유닛 RowName)
	UPROPERTY(BlueprintReadWrite, Category = "Purchase Slot")
	FName PurchaseID = NAME_None;

	// 이미지 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Purchase Slot")
	void SetPurchaseImage(UTexture2D* Texture);

	// 구매 아이템 텍스트 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Purchase Slot")
	void SetPurchaseItemText(const FString& Text);

	// 골드 비용 텍스트 설정 함수 (전략자원 없을 때 사용)
	UFUNCTION(BlueprintCallable, Category = "Purchase Slot")
	void SetGoldCostText(int32 GoldCost);

	// 전략자원 필요 시 표시 (GoldCostTxt/TimeImage 숨기고 Resource* 표시)
	UFUNCTION(BlueprintCallable, Category = "Purchase Slot")
	void SetStrategicResourceDisplay(int32 GoldCost, int32 RequiredAmount, UTexture2D* ResourceIconTexture);

protected:
	virtual void NativeConstruct() override;

	// 버튼 클릭 핸들러
	UFUNCTION()
	void OnPurchaseBtnClicked();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* PurchaseImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PurchaseItemTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* GoldCostTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* TimeImage = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PurchaseBtn = nullptr;

	// 전략자원 필요 시 표시용 (없으면 nullptr)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* ResourceGoldCostTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* ResourceTimeImage = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* ResourceStockTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* ResourceIconImg = nullptr;
};

