// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuUI.generated.h"

// 뒤로가기 버튼 클릭 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackButtonClicked);

UCLASS()
class CIVILIZATION_API UPauseMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 뒤로가기 버튼 클릭 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnBackButtonClicked OnBackButtonClickedDelegate;

private:
	UFUNCTION()
	void OnSaveButtonClicked();

	UFUNCTION()
	void OnSaveSlot1ButtonClicked();

	UFUNCTION()
	void OnSaveSlot2ButtonClicked();

	UFUNCTION()
	void OnSaveSlot3ButtonClicked();

	UFUNCTION()
	void OnSaveSlot4ButtonClicked();

	UFUNCTION()
	void OnSaveSlot5ButtonClicked();

	UFUNCTION()
	void OnTitleMenuButtonClicked();

	UFUNCTION()
	void OnGameEndButtonClicked();

	UFUNCTION()
	void OnBackButtonClicked();

	// 선택된 슬롯 인덱스 (1~5, 0은 미선택)
	int32 SelectedSlotIndex = 0;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveBtn;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot1Btn;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot2Btn;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot3Btn;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot4Btn;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot5Btn;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* TitleMenuBtn;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* GameEndBtn;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn;
};