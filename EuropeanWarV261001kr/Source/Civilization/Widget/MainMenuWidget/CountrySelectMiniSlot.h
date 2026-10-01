// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "CountrySelectMiniSlot.generated.h"

// 국가 미니 슬롯 클릭 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountrySelectMiniSlotClicked, class UCountrySelectMiniSlot*, ClickedSlot);

// 국가 미니 슬롯 호버 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountrySelectMiniSlotHovered, class UCountrySelectMiniSlot*, HoveredSlot);

// 국가 미니 슬롯 언호버 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountrySelectMiniSlotUnhovered);

UCLASS()
class CIVILIZATION_API UCountrySelectMiniSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 국가 이미지 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Country Select Mini Slot")
	void SetCountryImage(class UTexture2D* Texture);

	// RowName 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Country Select Mini Slot")
	void SetRowName(FName InRowName);

	// RowName 가져오기 함수
	UFUNCTION(BlueprintCallable, Category = "Country Select Mini Slot")
	FName GetRowName() const { return RowName; }

	// 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Country Select Mini Slot")
	FOnCountrySelectMiniSlotClicked OnCountrySelectMiniSlotClicked;

	// 호버 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Country Select Mini Slot")
	FOnCountrySelectMiniSlotHovered OnCountrySelectMiniSlotHovered;

	// 언호버 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Country Select Mini Slot")
	FOnCountrySelectMiniSlotUnhovered OnCountrySelectMiniSlotUnhovered;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CountryBtn = nullptr;

	// 버튼 클릭 핸들러
	UFUNCTION()
	void OnCountryBtnClicked();

	// 버튼 호버 시작 핸들러
	UFUNCTION()
	void OnCountryBtnHovered();

	// 버튼 호버 종료 핸들러
	UFUNCTION()
	void OnCountryBtnUnhovered();

private:
	// 국가 데이터 테이블의 RowName
	UPROPERTY()
	FName RowName = NAME_None;
};
