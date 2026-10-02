// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/Border.h"
#include "Components/UniformGridPanel.h"
#include "Components/Image.h"
#include "../MainMenuWidget/CountrySelectSlotUI.h"
#include "../MainMenuWidget/CountrySelectMiniSlot.h"
#include "../../Country/CountryStruct.h"
#include "../../World/WorldStruct.h"
#include "LobbyMenuUI.generated.h"

UCLASS()
class CIVILIZATION_API ULobbyMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 수명 주기 ==========

	// 슬라이더·국가 슬롯을 묶고 호스트면 세션을 만듭니다.
	virtual void NativeConstruct() override;
	// 로비 GameState·세션 델리게이트와 타이머를 해제합니다.
	virtual void NativeDestruct() override;

	// 복제된 월드 설정을 슬라이더에 반영합니다. 참가자 UI 동기화에 씁니다.
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ApplyWorldConfig(const FWorldConfig& WorldConfig);

	// 호스트 또는 참가자 슬롯에 국가 RowName을 넣습니다.
	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ApplyCountryToSlot(bool bHostSlot, FName CountryRowName);

protected:
	// ========== BindWidget / 버튼 ==========

	// 호스트만 보이는 게임 시작 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* StartBtn = nullptr;

	// 세션을 끊고 메인메뉴로 돌아가는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn = nullptr;

	// ========== BindWidget / 월드 슬라이더 ==========

	// 월드 반지름 슬라이더. 호스트만 조작합니다.
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

	// 호스트가 고르는 국가 슬롯
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCountrySelectSlotUI* HostCountrySelect = nullptr;

	// 참가자가 고르는 국가 슬롯
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCountrySelectSlotUI* ClientCountrySelect = nullptr;

	// 국가 목록을 여는 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* CountrySelectBrd = nullptr;

	// 국가 선택 보더를 여는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* OpenCountrySelectBrd = nullptr;

	// 국가 선택 보더를 닫는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* CloseCountrySelectBrd = nullptr;

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

	// 시작 실패·대기 메시지를 띄우는 경고 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* WarningTxt = nullptr;

	// ========== 버튼 / 세션 콜백 ==========

	// 인원·국가를 검사하고 월드 설정을 커밋한 뒤 인게임으로 갑니다.
	UFUNCTION()
	void OnStartButtonClicked();

	// 세션을 파괴한 뒤 메인메뉴로 돌아갑니다.
	UFUNCTION()
	void OnBackButtonClicked();

	// 세션 파괴가 끝나면 메인메뉴로 이동합니다.
	UFUNCTION()
	void OnDestroySessionComplete(bool bWasSuccessful);

	// 호스트 세션 생성 결과를 경고 텍스트에 보여 줍니다.
	UFUNCTION()
	void OnHostCreateSessionComplete(bool bWasSuccessful);

	// ========== 슬라이더 콜백 ==========

	// 월드 크기를 정수로 맞추고 호스트면 GameState에 올립니다.
	UFUNCTION()
	void OnWorldSizeSliderValueChanged(float Value);

	// 숲 비율을 맞추고 호스트면 GameState에 올립니다.
	UFUNCTION()
	void OnForestRatioSliderValueChanged(float Value);

	// 온대 비율을 맞추고 호스트면 GameState에 올립니다.
	UFUNCTION()
	void OnTemperateRatioSliderValueChanged(float Value);

	// 평지 비율을 맞추고 호스트면 GameState에 올립니다.
	UFUNCTION()
	void OnPlainRatioSliderValueChanged(float Value);

	// 사막·툰드라 비율을 맞추고 호스트면 GameState에 올립니다.
	UFUNCTION()
	void OnDesertRatioSliderValueChanged(float Value);

	// 언덕·산 비율을 맞추고 호스트면 GameState에 올립니다.
	UFUNCTION()
	void OnHillRatioSliderValueChanged(float Value);

	// ========== 국가 슬롯 콜백 ==========

	// 내 슬롯을 눌렀을 때만 국가 선택 보더를 엽니다.
	UFUNCTION()
	void OnCountrySelectSlotClicked(class UCountrySelectSlotUI* ClickedSlot);

	// 미니 슬롯 호버 시 미리보기 이미지·이름·색을 바꿉니다.
	UFUNCTION()
	void OnCountrySelectMiniSlotHovered(class UCountrySelectMiniSlot* HoveredSlot);

	// 미니 슬롯에서 손을 떼면 미리보기를 NoSelect로 되돌립니다.
	UFUNCTION()
	void OnCountrySelectMiniSlotUnhovered();

	// 미니 슬롯을 누르면 내 슬롯 국가를 요청합니다.
	UFUNCTION()
	void OnCountrySelectMiniSlotClicked(class UCountrySelectMiniSlot* ClickedSlot);

private:
	// ========== 로비 헬퍼 ==========

	// 이 기기가 로비 호스트인지 반환합니다.
	bool IsLobbyHost() const;
	// 호스트면 호스트 슬롯, 아니면 참가자 슬롯을 반환합니다.
	UCountrySelectSlotUI* GetLocalCountrySlot() const;
	// 호스트만 슬라이더와 시작 버튼을 켜 둡니다.
	void ApplyInteractableRules();
	// 국가 슬롯 클릭 델리게이트를 묶습니다.
	void BindCountrySlot(UCountrySelectSlotUI* CountrySlot);
	// 슬롯을 NoSelect 데이터로 초기화합니다.
	void InitializeSlotWithNoSelect(UCountrySelectSlotUI* TargetSlot);
	// 국가 데이터 테이블로 미니 슬롯 그리드를 채웁니다.
	void InitializeCountrySelectMiniSlots();
	// 호버 미리보기를 NoSelect로 맞춥니다.
	void InitializeHoveredCountryUI();
	// 마지막으로 연 슬롯에 국가 데이터를 넣습니다.
	void ApplyCountryDataToSelectedSlot(FName RowName);
	// 지정 슬롯에 국가 이미지·이름·RowName을 넣습니다.
	void ApplyCountryDataToSlot(UCountrySelectSlotUI* TargetSlot, FName RowName);
	// 호스트 또는 참가자 슬롯이 아직 비어 있는지 반환합니다.
	bool HasUnselectedCountry() const;
	// 슬라이더 값으로 FWorldConfig를 만듭니다. 멀티는 항상 2인입니다.
	FWorldConfig BuildWorldConfigFromSliders() const;
	// 경고 텍스트를 보여 줍니다.
	void SetWarningText(const FString& Message);
	// 경고 텍스트를 숨깁니다.
	void HideWarningText();
	// 메인메뉴 레벨로 돌아갑니다.
	void ReturnToMainMenu();
	// 호스트이고 아직 방이 없으면 세션을 만듭니다.
	void TryCreateHostSession();
	// 멀티플레이 세션 서브시스템을 반환합니다.
	class UMultiplayerSessionSubsystem* GetSessionSubsystem() const;
	// 로비 GameState를 반환합니다.
	class ALobbyGameState* GetLobbyGameState() const;
	// GameState가 생기면 상태 변경을 묶고, 없으면 타이머로 재시도합니다.
	void BindLobbyGameState();
	// 호스트일 때 슬라이더 월드 설정을 GameState에 올립니다.
	void PushWorldConfigIfHost();
	// 호스트는 GameState에, 참가자는 서버 RPC로 내 국가를 올립니다.
	void RequestLocalCountry(FName CountryRowName);

	// 복제된 로비 상태(월드 설정·국가)를 UI에 반영합니다.
	UFUNCTION()
	void OnLobbyStateChanged();
	// 정수 슬라이더를 반올림하고 라벨에 숫자를 씁니다.
	void SnapIntegerSlider(USlider* Slider, float Value, UTextBlock* LabelTxt);
	// 비율 슬라이더를 0.1 단위로 맞추고 퍼센트 라벨을 갱신합니다.
	void SnapRatioSlider(USlider* Slider, float Value, UTextBlock* LabelTxt, bool bInvertPercent);

	// 국가 목록을 연 마지막 슬롯
	UPROPERTY()
	UCountrySelectSlotUI* LastSelectedSlot = nullptr;

	// GameState 바인딩을 재시도하는 타이머
	FTimerHandle BindLobbyGameStateTimerHandle;

	// 복제 설정을 넣는 중이면 슬라이더가 GameState로 다시 올리지 않습니다.
	bool bIsApplyingWorldConfig = false;
};
