// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PauseMenuUI.generated.h"

// 뒤로가기 버튼을 눌렀을 때 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackButtonClicked);

UCLASS()
class CIVILIZATION_API UPauseMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 뒤로가기 버튼을 누르면 방송하는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnBackButtonClicked OnBackButtonClickedDelegate;

private:
	// ========== 버튼 핸들러 ==========

	// 저장 버튼을 누르면 선택한 슬롯에 저장합니다.
	UFUNCTION()
	void OnSaveButtonClicked();

	// 세이브 슬롯 1을 선택합니다.
	UFUNCTION()
	void OnSaveSlot1ButtonClicked();

	// 세이브 슬롯 2를 선택합니다.
	UFUNCTION()
	void OnSaveSlot2ButtonClicked();

	// 세이브 슬롯 3을 선택합니다.
	UFUNCTION()
	void OnSaveSlot3ButtonClicked();

	// 세이브 슬롯 4를 선택합니다.
	UFUNCTION()
	void OnSaveSlot4ButtonClicked();

	// 세이브 슬롯 5를 선택합니다.
	UFUNCTION()
	void OnSaveSlot5ButtonClicked();

	// 타이틀 메뉴로 돌아갑니다.
	UFUNCTION()
	void OnTitleMenuButtonClicked();

	// 게임을 종료합니다.
	UFUNCTION()
	void OnGameEndButtonClicked();

	// 일시정지 메뉴를 닫도록 델리게이트를 방송합니다.
	UFUNCTION()
	void OnBackButtonClicked();

	// 선택한 세이브 슬롯을 삭제합니다.
	UFUNCTION()
	void OnDeleteButtonClicked();

	// 선택된 슬롯 인덱스. 1~5이며 0은 미선택입니다.
	int32 SelectedSlotIndex = 0;

protected:
	// ========== BindWidget ==========

	// 선택한 슬롯에 저장하는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveBtn;

	// 세이브 슬롯 1 선택 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot1Btn;

	// 세이브 슬롯 2 선택 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot2Btn;

	// 세이브 슬롯 3 선택 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot3Btn;

	// 세이브 슬롯 4 선택 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot4Btn;

	// 세이브 슬롯 5 선택 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot5Btn;

	// 타이틀 메뉴로 돌아가는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* TitleMenuBtn;

	// 게임을 종료하는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* GameEndBtn;

	// 일시정지 메뉴를 닫는 뒤로가기 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn;

	// 선택한 세이브 슬롯을 지우는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* DeleteBtn;

	// 선택한 슬롯의 저장 정보를 보여주는 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* SlotExplainTxt;

private:
	// 선택한 슬롯의 저장 정보 텍스트를 갱신합니다.
	void UpdateSlotInfo(int32 SlotIndex);
};
