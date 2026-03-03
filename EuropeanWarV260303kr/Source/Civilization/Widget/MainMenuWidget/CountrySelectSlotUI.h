// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "CountrySelectSlotUI.generated.h"

// 국가 선택 슬롯 클릭 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountrySelectSlotClicked, class UCountrySelectSlotUI*, Slot);

UCLASS()
class CIVILIZATION_API UCountrySelectSlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// UI 요소 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Country Select Slot")
	void SetCountryImage(class UTexture2D* Texture);

	UFUNCTION(BlueprintCallable, Category = "Country Select Slot")
	void SetCountryText(const FString& Text);

	// RowName 설정 및 가져오기
	UFUNCTION(BlueprintCallable, Category = "Country Select Slot")
	void SetRowName(FName InRowName);

	UFUNCTION(BlueprintCallable, Category = "Country Select Slot")
	FName GetRowName() const { return RowName; }

	// 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Country Select Slot")
	FOnCountrySelectSlotClicked OnCountrySelectSlotClicked;

private:
	// 현재 선택된 국가의 RowName
	FName RowName = NAME_None;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* CountryTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SelectBtn = nullptr;

	// 버튼 클릭 핸들러
	UFUNCTION()
	void OnSelectBtnClicked();
};
