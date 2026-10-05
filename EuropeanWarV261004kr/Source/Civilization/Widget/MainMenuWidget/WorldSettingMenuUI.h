// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Components/Border.h"
#include "Components/UniformGridPanel.h"
#include "Components/Image.h"
#include "CountrySelectSlotUI.h"
#include "CountrySelectMiniSlot.h"
#include "../../Country/CountryStruct.h"
#include "WorldSettingMenuUI.generated.h"

// 월드 설정을 닫고 메인 메뉴로 돌아갈 때 씁니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldSettingMenuBackButtonClicked);

UCLASS()
class CIVILIZATION_API UWorldSettingMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 수명 주기 ==========

	// 슬라이더·국가 슬롯을 묶고 미니 슬롯 그리드를 채웁니다.
	virtual void NativeConstruct() override;

	// 메인 메뉴가 월드 설정을 닫을 때 듣는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnWorldSettingMenuBackButtonClicked OnBackButtonClickedDelegate;

protected:
	// ========== BindWidget / 버튼 ==========

	// 설정을 GI에 넣고 로딩을 거쳐 인게임으로 가는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* StartBtn = nullptr;

	// 월드 설정을 닫고 메인 메뉴로 돌아가는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn = nullptr;

	// ========== BindWidget / 월드 슬라이더 ==========

	// 사람+AI 슬롯 수 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* PlayerCountSlider = nullptr;

	// 플레이어 수 숫자 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerCountTxt = nullptr;

	// 월드 반지름 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* WorldSizeSlider = nullptr;

	// 월드 반지름 숫자 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* WorldSizeTxt = nullptr;

	// 숲 비율 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* ForestRatioSlider = nullptr;

	// 숲 비율 퍼센트 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ForestRatioTxt = nullptr;

	// 온대 비율 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* TemperateRatioSlider = nullptr;

	// 온대 비율 퍼센트 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TemperateRatioTxt = nullptr;

	// 평지 비율 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* PlainRatioSlider = nullptr;

	// 평지 비율 퍼센트 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlainRatioTxt = nullptr;

	// 사막·툰드라 배분 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* DesertRatioSlider = nullptr;

	// 사막 비율 퍼센트 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DesertRatioTxt = nullptr;

	// 툰드라 비율 퍼센트 텍스트. 사막 슬라이더의 반대쪽입니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TundraRatioTxt = nullptr;

	// 언덕·산 배분 슬라이더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* HillRatioSlider = nullptr;

	// 언덕 비율 퍼센트 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* HillRatioTxt = nullptr;

	// 산 비율 퍼센트 텍스트. 언덕 슬라이더의 반대쪽입니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* MountainRatioTxt = nullptr;

	// ========== BindWidget / 국가 선택 ==========

	// 사람 플레이어(슬롯 0) 국가 슬롯
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCountrySelectSlotUI* PlayerSelect = nullptr;

	// AI 국가 슬롯을 담는 세로 박스. 플레이어 수에 따라 개수가 바뀝니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* AIPlayerSelectVB = nullptr;

	// 국가 목록을 여는 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* CountrySelectBrd = nullptr;

	// 국가 선택 보더를 여는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* OpenCountrySelectBrd = nullptr;
	// 국가 선택 보더를 닫는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* CloseCountrySelectBrd = nullptr;
	// 국가 선택 보더를 리셋하는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* ResetCountrySelectBrd = nullptr;

	// 국가 미니 슬롯을 담는 그리드
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UUniformGridPanel* CountrySelectUGP = nullptr;

	// 호버 중인 국가의 왕/국기 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* HoveredCountryImg = nullptr;

	// 호버 중인 국가 이름
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* HoveredCountryTxt = nullptr;

	// 호버 중인 국가 색
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* HoveredCountryColorBrd = nullptr;

	// 시작 실패 메시지를 띄우는 경고 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* WarningTxt = nullptr;

	// ========== 버튼 / 슬라이더 콜백 ==========

	// 국가가 다 골라졌으면 설정을 GI에 넣고 로딩으로 갑니다.
	UFUNCTION()
	void OnStartButtonClicked();

	// 뒤로가기 델리게이트를 브로드캐스트합니다.
	UFUNCTION()
	void OnBackButtonClicked();

	// 플레이어 수를 정수로 맞추고 AI 슬롯 개수를 갱신합니다.
	UFUNCTION()
	void OnPlayerCountSliderValueChanged(float Value);

	// 월드 크기 숫자를 갱신합니다.
	UFUNCTION()
	void OnWorldSizeSliderValueChanged(float Value);

	// 숲 비율 퍼센트를 갱신합니다.
	UFUNCTION()
	void OnForestRatioSliderValueChanged(float Value);

	// 온대 비율 퍼센트를 갱신합니다.
	UFUNCTION()
	void OnTemperateRatioSliderValueChanged(float Value);

	// 평지 비율 퍼센트를 갱신합니다.
	UFUNCTION()
	void OnPlainRatioSliderValueChanged(float Value);

	// 사막·툰드라 비율 퍼센트를 갱신합니다.
	UFUNCTION()
	void OnDesertRatioSliderValueChanged(float Value);

	// 언덕·산 비율 퍼센트를 갱신합니다.
	UFUNCTION()
	void OnHillRatioSliderValueChanged(float Value);

	// ========== 국가 슬롯 ==========

	// 사람 플레이어 슬롯을 넣고 클릭 델리게이트를 묶습니다.
	UFUNCTION(BlueprintCallable, Category = "Player Select")
	void SetPlayerSelect(class UCountrySelectSlotUI* InPlayerSelect);

	// 누른 슬롯을 기억하고 국가 선택 보더를 엽니다.
	UFUNCTION()
	void OnCountrySelectSlotClicked(class UCountrySelectSlotUI* ClickedSlot);

	// 미니 슬롯 호버 시 미리보기 이미지·이름·색을 바꿉니다.
	UFUNCTION()
	void OnCountrySelectMiniSlotHovered(class UCountrySelectMiniSlot* HoveredSlot);

	// 미니 슬롯에서 손을 떼면 미리보기를 NoSelect로 되돌립니다.
	UFUNCTION()
	void OnCountrySelectMiniSlotUnhovered();

	// 미니 슬롯을 누르면 마지막으로 연 슬롯에 국가를 넣습니다.
	UFUNCTION()
	void OnCountrySelectMiniSlotClicked(class UCountrySelectMiniSlot* ClickedSlot);

private:
	// ========== 국가 / AI 슬롯 헬퍼 ==========

	// 플레이어 수에 맞춰 AI 슬롯을 만들거나 지웁니다.
	void UpdateAIPlayerSlots();

	// AI 슬롯 클릭 델리게이트를 묶습니다.
	void BindAIPlayerSlotDelegates();

	// 슬롯을 NoSelect 데이터로 초기화합니다.
	void InitializeSlotWithNoSelect(class UCountrySelectSlotUI* TargetSlot);

	// 국가 데이터 테이블로 미니 슬롯 그리드를 채웁니다.
	void InitializeCountrySelectMiniSlots();

	// 호버 미리보기를 NoSelect로 맞춥니다.
	void InitializeHoveredCountryUI();

	// 마지막으로 연 슬롯에 국가 데이터를 넣습니다.
	void ApplyCountryDataToSelectedSlot(FName RowName);

	// PlayerSelect 다음 AI 슬롯 순서로 국가 RowName 배열을 모읍니다.
	TArray<FName> CollectCountryNames() const;

	// 국가 목록을 연 마지막 슬롯 (사람 또는 AI)
	UPROPERTY()
	class UCountrySelectSlotUI* LastSelectedSlot = nullptr;
};
