// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LoadMenuUI.generated.h"

// 로드 메뉴를 닫고 메인 메뉴로 돌아갈 때 씁니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadMenuBackButtonClicked);

UCLASS()
class CIVILIZATION_API ULoadMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 수명 주기 ==========

	// 슬롯·로드·삭제 버튼을 묶고 설명·경고 텍스트를 숨깁니다.
	virtual void NativeConstruct() override;

	// 메인 메뉴가 로드 메뉴를 닫을 때 듣는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnLoadMenuBackButtonClicked OnBackButtonClickedDelegate;

private:
	// ========== 슬롯 선택 ==========

	// 슬롯 1을 고르고 저장 정보를 보여 줍니다.
	UFUNCTION()
	void OnSaveSlot1ButtonClicked();

	// 슬롯 2를 고르고 저장 정보를 보여 줍니다.
	UFUNCTION()
	void OnSaveSlot2ButtonClicked();

	// 슬롯 3을 고르고 저장 정보를 보여 줍니다.
	UFUNCTION()
	void OnSaveSlot3ButtonClicked();

	// 슬롯 4를 고르고 저장 정보를 보여 줍니다.
	UFUNCTION()
	void OnSaveSlot4ButtonClicked();

	// 슬롯 5를 고르고 저장 정보를 보여 줍니다.
	UFUNCTION()
	void OnSaveSlot5ButtonClicked();

	// 고른 슬롯의 세이브 파일을 지웁니다.
	UFUNCTION()
	void OnDeleteButtonClicked();

	// 선택된 슬롯 인덱스 (1~5, 0은 미선택)
	int32 SelectedSlotIndex = 0;

protected:
	// ========== BindWidget ==========

	// 세이브 슬롯 1 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot1Btn = nullptr;

	// 세이브 슬롯 2 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot2Btn = nullptr;

	// 세이브 슬롯 3 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot3Btn = nullptr;

	// 세이브 슬롯 4 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot4Btn = nullptr;

	// 세이브 슬롯 5 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SaveSlot5Btn = nullptr;

	// 고른 슬롯을 불러와 로딩 맵으로 가는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* LoadBtn = nullptr;

	// 로드 메뉴를 닫고 메인 메뉴로 돌아가는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn = nullptr;

	// 고른 슬롯의 세이브를 지우는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* DeleteBtn = nullptr;

	// 저장 일시·국가·라운드를 보여 주는 설명 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* SlotExplainTxt = nullptr;

	// 슬롯 미선택·빈 슬롯 경고 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* WarningTxt = nullptr;

	// 고른 슬롯이 있으면 GI에 넣고 로딩을 거쳐 인게임으로 갑니다.
	UFUNCTION()
	void OnLoadButtonClicked();

	// 설명·경고를 숨기고 뒤로가기 델리게이트를 브로드캐스트합니다.
	UFUNCTION()
	void OnBackButtonClicked();

private:
	// 슬롯 세이브가 있으면 저장 일시·국가·라운드를 설명 텍스트에 씁니다.
	void UpdateSlotInfo(int32 SlotIndex);
};
