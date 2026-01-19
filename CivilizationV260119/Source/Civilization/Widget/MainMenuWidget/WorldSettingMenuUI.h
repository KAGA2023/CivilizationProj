// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Border.h"
#include "Components/UniformGridPanel.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CountrySelectSlotUI.h"
#include "CountrySelectMiniSlot.h"
#include "../../Country/CountryStruct.h"
#include "WorldSettingMenuUI.generated.h"

// 뒤로가기 버튼 클릭 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldSettingMenuBackButtonClicked);

UCLASS()
class CIVILIZATION_API UWorldSettingMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 뒤로가기 버튼 클릭 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnWorldSettingMenuBackButtonClicked OnBackButtonClickedDelegate;

protected:
	// 시작 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* StartBtn = nullptr;

	// 뒤로가기 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn = nullptr;

	// 플레이어 수 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* PlayerCountSlider = nullptr;

	// 플레이어 수 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerCountTxt = nullptr;

	// 월드 크기 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* WorldSizeSlider = nullptr;

	// 월드 크기 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* WorldSizeTxt = nullptr;

	// 숲 비율 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* ForestRatioSlider = nullptr;

	// 숲 비율 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ForestRatioTxt = nullptr;

	// 온대 비율 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* TemperateRatioSlider = nullptr;

	// 온대 비율 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TemperateRatioTxt = nullptr;

	// 사막과 툰드라 비율 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DesertAndTundraRatioTxt = nullptr;

	// 평지 비율 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* PlainRatioSlider = nullptr;

	// 평지 비율 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlainRatioTxt = nullptr;

	// 언덕과 산 비율 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* HillAndMountainRatioTxt = nullptr;

	// 사막 비율 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* DesertRatioSlider = nullptr;

	// 사막 비율 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DesertRatioTxt = nullptr;

	// 툰드라 비율 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TundraRatioTxt = nullptr;

	// 언덕 비율 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* HillRatioSlider = nullptr;

	// 언덕 비율 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* HillRatioTxt = nullptr;

	// 산 비율 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* MountainRatioTxt = nullptr;

	// 플레이어 선택 슬롯
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCountrySelectSlotUI* PlayerSelect = nullptr;

	// AI 플레이어 선택 VerticalBox
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* AIPlayerSelectVB = nullptr;

	// 국가 선택 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* CountrySelectBrd = nullptr;

	// 국가 선택 유니폼 그리드 패널
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UUniformGridPanel* CountrySelectUGP = nullptr;

	// 호버된 국가 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* HoveredCountryImg = nullptr;

	// 호버된 국가 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* HoveredCountryTxt = nullptr;

	// 호버된 국가 색상 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* HoveredCountryColorBrd = nullptr;

	// 경고 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* WarningTxt = nullptr;

	// 시작 버튼 클릭 핸들러
	UFUNCTION()
	void OnStartButtonClicked();

	// 뒤로가기 버튼 클릭 핸들러
	UFUNCTION()
	void OnBackButtonClicked();

	// 플레이어 수 슬라이더 값 변경 핸들러
	UFUNCTION()
	void OnPlayerCountSliderValueChanged(float Value);

	// 월드 크기 슬라이더 값 변경 핸들러
	UFUNCTION()
	void OnWorldSizeSliderValueChanged(float Value);

	// 숲 비율 슬라이더 값 변경 핸들러
	UFUNCTION()
	void OnForestRatioSliderValueChanged(float Value);

	// 온대 비율 슬라이더 값 변경 핸들러
	UFUNCTION()
	void OnTemperateRatioSliderValueChanged(float Value);

	// 평지 비율 슬라이더 값 변경 핸들러
	UFUNCTION()
	void OnPlainRatioSliderValueChanged(float Value);

	// 사막 비율 슬라이더 값 변경 핸들러
	UFUNCTION()
	void OnDesertRatioSliderValueChanged(float Value);

	// 언덕 비율 슬라이더 값 변경 핸들러
	UFUNCTION()
	void OnHillRatioSliderValueChanged(float Value);

	// PlayerSelect 위젯 설정 함수 (블루프린트에서 호출 가능)
	UFUNCTION(BlueprintCallable, Category = "Player Select")
	void SetPlayerSelect(class UCountrySelectSlotUI* InPlayerSelect);

	// 국가 선택 슬롯 클릭 핸들러
	UFUNCTION()
	void OnCountrySelectSlotClicked(class UCountrySelectSlotUI* ClickedSlot);

	// 국가 선택 미니 슬롯 호버 핸들러
	UFUNCTION()
	void OnCountrySelectMiniSlotHovered(class UCountrySelectMiniSlot* HoveredSlot);

	// 국가 선택 미니 슬롯 언호버 핸들러
	UFUNCTION()
	void OnCountrySelectMiniSlotUnhovered();

	// 국가 선택 미니 슬롯 클릭 핸들러
	UFUNCTION()
	void OnCountrySelectMiniSlotClicked(class UCountrySelectMiniSlot* ClickedSlot);

private:
	// AI 플레이어 슬롯 업데이트 함수
	void UpdateAIPlayerSlots();

	// AI 플레이어 슬롯의 델리게이트 바인딩 함수
	void BindAIPlayerSlotDelegates();

	// 슬롯을 NoSelect 데이터로 초기화하는 함수
	void InitializeSlotWithNoSelect(class UCountrySelectSlotUI* TargetSlot);

	// 국가 선택 미니 슬롯 그리드 초기화 함수
	void InitializeCountrySelectMiniSlots();

	// 호버된 국가 UI 기본값 초기화 함수 (NoSelect)
	void InitializeHoveredCountryUI();

	// 선택된 슬롯에 국가 데이터 설정하는 함수
	void ApplyCountryDataToSelectedSlot(FName RowName);

	// 국가 배열 수집 함수 (PlayerSelect + AIPlayerSelectVB 순서대로)
	TArray<FName> CollectCountryNames() const;

	// 마지막으로 클릭한 CountrySelectSlotUI (PlayerSelect 또는 AIPlayerSelectVB 내부)
	UPROPERTY()
	class UCountrySelectSlotUI* LastSelectedSlot = nullptr;
};
