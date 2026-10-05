// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "CountrySelectSlotUI.generated.h"

// 슬롯을 누르면 부모 메뉴가 국가 목록 보더를 엽니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountrySelectSlotClicked, class UCountrySelectSlotUI*, Slot);

UCLASS()
class CIVILIZATION_API UCountrySelectSlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 수명 주기 ==========

	// 선택 버튼을 묶습니다.
	virtual void NativeConstruct() override;

	// 슬롯에 국가 이미지를 넣습니다.
	UFUNCTION(BlueprintCallable, Category = "Country Select Slot")
	void SetCountryImage(class UTexture2D* Texture);

	// 슬롯에 국가 이름을 넣습니다.
	UFUNCTION(BlueprintCallable, Category = "Country Select Slot")
	void SetCountryText(const FString& Text);

	// 국가 데이터 테이블 RowName을 저장합니다.
	UFUNCTION(BlueprintCallable, Category = "Country Select Slot")
	void SetRowName(FName InRowName);

	// 이 슬롯의 국가 RowName을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Country Select Slot")
	FName GetRowName() const { return RowName; }

	// 이 슬롯을 눌러 국가 목록을 열 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Country Select Slot")
	FOnCountrySelectSlotClicked OnCountrySelectSlotClicked;

private:
	// 현재 선택된 국가의 RowName. NoSelect면 아직 고르지 않은 것입니다.
	FName RowName = NAME_None;

protected:
	// ========== BindWidget ==========

	// 고른 국가 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	// 고른 국가 이름
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* CountryTxt = nullptr;

	// 국가 목록을 열기 위해 슬롯을 누르는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SelectBtn = nullptr;

	// 클릭 델리게이트를 브로드캐스트합니다.
	UFUNCTION()
	void OnSelectBtnClicked();
};
