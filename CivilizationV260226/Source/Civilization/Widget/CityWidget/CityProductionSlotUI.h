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

UCLASS()
class CIVILIZATION_API UCityProductionSlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UCityProductionSlotUI(const FObjectInitializer& ObjectInitializer);

	// 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Production Slot")
	FOnProductionSlotClicked OnProductionSlotClicked;

	// 생산 ID (건물 또는 유닛 RowName)
	UPROPERTY(BlueprintReadWrite, Category = "Production Slot")
	FName ProductionID = NAME_None;

	// 이미지 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Production Slot")
	void SetProductionImage(UTexture2D* Texture);

	// 생산 아이템 텍스트 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Production Slot")
	void SetProductionItemText(const FString& Text);

	// 턴 수 텍스트 설정 함수 (전략자원 없을 때 사용)
	UFUNCTION(BlueprintCallable, Category = "Production Slot")
	void SetTurnText(int32 Turns);

	// 전략자원 필요 시 표시 (TurnTxt/TimeImage 숨기고 Resource* 표시)
	UFUNCTION(BlueprintCallable, Category = "Production Slot")
	void SetStrategicResourceDisplay(int32 Turns, int32 RequiredAmount, UTexture2D* ResourceIconTexture);

protected:
	virtual void NativeConstruct() override;

	// 버튼 클릭 핸들러
	UFUNCTION()
	void OnStartProductionBtnClicked();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ProductionImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ProductionItemTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TurnTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* TimeImage = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* StartProductionBtn = nullptr;

	// 전략자원 필요 시 표시용 (없으면 nullptr)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* ResourceTurnTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* ResourceTimeImage = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* ResourceStockTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* ResourceIconImg = nullptr;
};


