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
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ApplyWorldConfig(const FWorldConfig& WorldConfig);

	UFUNCTION(BlueprintCallable, Category = "Lobby")
	void ApplyCountryToSlot(bool bHostSlot, FName CountryRowName);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* StartBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* WorldSizeSlider = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* WorldSizeTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* ForestRatioSlider = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ForestRatioTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* TemperateRatioSlider = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TemperateRatioTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* PlainRatioSlider = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlainRatioTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* DesertRatioSlider = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DesertRatioTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TundraRatioTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class USlider* HillRatioSlider = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* HillRatioTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* MountainRatioTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCountrySelectSlotUI* HostCountrySelect = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCountrySelectSlotUI* ClientCountrySelect = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* CountrySelectBrd = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* OpenCountrySelectBrd = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* CloseCountrySelectBrd = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UUniformGridPanel* CountrySelectUGP = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* HoveredCountryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* HoveredCountryTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* HoveredCountryColorBrd = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* WarningTxt = nullptr;

	UFUNCTION()
	void OnStartButtonClicked();

	UFUNCTION()
	void OnBackButtonClicked();

	UFUNCTION()
	void OnDestroySessionComplete(bool bWasSuccessful);

	UFUNCTION()
	void OnHostCreateSessionComplete(bool bWasSuccessful);

	UFUNCTION()
	void OnWorldSizeSliderValueChanged(float Value);

	UFUNCTION()
	void OnForestRatioSliderValueChanged(float Value);

	UFUNCTION()
	void OnTemperateRatioSliderValueChanged(float Value);

	UFUNCTION()
	void OnPlainRatioSliderValueChanged(float Value);

	UFUNCTION()
	void OnDesertRatioSliderValueChanged(float Value);

	UFUNCTION()
	void OnHillRatioSliderValueChanged(float Value);

	UFUNCTION()
	void OnCountrySelectSlotClicked(class UCountrySelectSlotUI* ClickedSlot);

	UFUNCTION()
	void OnCountrySelectMiniSlotHovered(class UCountrySelectMiniSlot* HoveredSlot);

	UFUNCTION()
	void OnCountrySelectMiniSlotUnhovered();

	UFUNCTION()
	void OnCountrySelectMiniSlotClicked(class UCountrySelectMiniSlot* ClickedSlot);

private:
	bool IsLobbyHost() const;
	UCountrySelectSlotUI* GetLocalCountrySlot() const;
	void ApplyInteractableRules();
	void BindCountrySlot(UCountrySelectSlotUI* CountrySlot);
	void InitializeSlotWithNoSelect(UCountrySelectSlotUI* TargetSlot);
	void InitializeCountrySelectMiniSlots();
	void InitializeHoveredCountryUI();
	void ApplyCountryDataToSelectedSlot(FName RowName);
	void ApplyCountryDataToSlot(UCountrySelectSlotUI* TargetSlot, FName RowName);
	bool HasUnselectedCountry() const;
	FWorldConfig BuildWorldConfigFromSliders() const;
	void SetWarningText(const FString& Message);
	void HideWarningText();
	void ReturnToMainMenu();
	void TryCreateHostSession();
	class UMultiplayerSessionSubsystem* GetSessionSubsystem() const;
	class ALobbyGameState* GetLobbyGameState() const;
	void BindLobbyGameState();
	void PushWorldConfigIfHost();
	void RequestLocalCountry(FName CountryRowName);

	UFUNCTION()
	void OnLobbyStateChanged();
	void SnapIntegerSlider(USlider* Slider, float Value, UTextBlock* LabelTxt);
	void SnapRatioSlider(USlider* Slider, float Value, UTextBlock* LabelTxt, bool bInvertPercent);

	UPROPERTY()
	UCountrySelectSlotUI* LastSelectedSlot = nullptr;

	FTimerHandle BindLobbyGameStateTimerHandle;

	bool bIsApplyingWorldConfig = false;
};
