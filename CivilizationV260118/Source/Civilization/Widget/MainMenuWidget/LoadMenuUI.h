// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadMenuUI.generated.h"

// 뒤로가기 버튼 클릭 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadMenuBackButtonClicked);

UCLASS()
class CIVILIZATION_API ULoadMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 뒤로가기 버튼 클릭 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnLoadMenuBackButtonClicked OnBackButtonClickedDelegate;

private:
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

	// 선택된 슬롯 인덱스 (1~5, 0은 미선택)
	int32 SelectedSlotIndex = 0;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot1Btn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot2Btn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot3Btn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot4Btn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot5Btn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* LoadBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn = nullptr;

	// 로드 버튼 클릭 핸들러
	UFUNCTION()
	void OnLoadButtonClicked();

	// 뒤로가기 버튼 클릭 핸들러
	UFUNCTION()
	void OnBackButtonClicked();
};
