// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "CountrySelectMiniSlot.generated.h"

// 미니 슬롯을 누르면 부모 메뉴가 국가를 고릅니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountrySelectMiniSlotClicked, class UCountrySelectMiniSlot*, ClickedSlot);

// 미니 슬롯 호버 시 부모 메뉴가 미리보기를 바꿉니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountrySelectMiniSlotHovered, class UCountrySelectMiniSlot*, HoveredSlot);

// 미니 슬롯에서 손을 떼면 부모 메뉴가 미리보기를 되돌립니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCountrySelectMiniSlotUnhovered);

UCLASS()
class CIVILIZATION_API UCountrySelectMiniSlot : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 수명 주기 ==========

	// 클릭·호버·언호버를 버튼에 묶습니다.
	virtual void NativeConstruct() override;

	// 미니 슬롯 국기 이미지를 넣습니다.
	UFUNCTION(BlueprintCallable, Category = "Country Select Mini Slot")
	void SetCountryImage(class UTexture2D* Texture);

	// 국가 데이터 테이블 RowName을 저장합니다.
	UFUNCTION(BlueprintCallable, Category = "Country Select Mini Slot")
	void SetRowName(FName InRowName);

	// 이 슬롯의 국가 RowName을 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Country Select Mini Slot")
	FName GetRowName() const { return RowName; }

	// 이 슬롯을 골랐을 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Country Select Mini Slot")
	FOnCountrySelectMiniSlotClicked OnCountrySelectMiniSlotClicked;

	// 이 슬롯 위에 마우스를 올렸을 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Country Select Mini Slot")
	FOnCountrySelectMiniSlotHovered OnCountrySelectMiniSlotHovered;

	// 이 슬롯에서 마우스를 뗐을 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Country Select Mini Slot")
	FOnCountrySelectMiniSlotUnhovered OnCountrySelectMiniSlotUnhovered;

protected:
	// ========== BindWidget ==========

	// 국가 국기 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	// 미니 슬롯 전체를 누르는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CountryBtn = nullptr;

	// 클릭 델리게이트를 브로드캐스트합니다.
	UFUNCTION()
	void OnCountryBtnClicked();

	// 호버 델리게이트를 브로드캐스트합니다.
	UFUNCTION()
	void OnCountryBtnHovered();

	// 언호버 델리게이트를 브로드캐스트합니다.
	UFUNCTION()
	void OnCountryBtnUnhovered();

private:
	// 이 슬롯이 가리키는 국가 데이터 테이블 RowName
	UPROPERTY()
	FName RowName = NAME_None;
};
