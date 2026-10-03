// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "MainMenuUI.generated.h"

class UWorldSettingMenuUI;
class UMultiplayerMenuUI;

// 서브 메뉴에서 메인 메뉴 캔버스로 돌아올 때 씁니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackToMainMenu);

/**
 * 메인 메뉴 UI 위젯
 */
UCLASS()
class CIVILIZATION_API UMainMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 버튼을 묶고 서브 메뉴를 숨긴 뒤 로컬 슬롯을 싱글로 되돌립니다.
	virtual void NativeConstruct() override;

protected:
	// ========== BindWidget ==========

	// 메인 메뉴 버튼이 올라가는 캔버스. 서브 메뉴를 열면 숨깁니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCanvasPanel* MainMenuCanvas = nullptr;

	// 싱글 플레이 월드 설정 화면을 여는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PlayBtn = nullptr;

	// 게임을 종료하는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* EndBtn = nullptr;

	// 세이브 로드 화면을 여는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* LoadBtn = nullptr;

	// 크레딧 화면을 여는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CreditBtn = nullptr;

	// 멀티플레이 메뉴를 여는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MultiplayerBtn = nullptr;

	// ========== 버튼 콜백 ==========

	// 월드 설정 메뉴를 열고 메인 캔버스를 숨깁니다.
	UFUNCTION()
	void OnPlayButtonClicked();

	// 게임을 종료합니다.
	UFUNCTION()
	void OnEndButtonClicked();

	// 로드 메뉴를 열고 메인 캔버스를 숨깁니다.
	UFUNCTION()
	void OnLoadButtonClicked();

	// 크레딧 메뉴를 열고 메인 캔버스를 숨깁니다.
	UFUNCTION()
	void OnCreditButtonClicked();

	// 멀티플레이 메뉴를 열고 메인 캔버스를 숨깁니다.
	UFUNCTION()
	void OnMultiplayerButtonClicked();

	// ========== 서브 메뉴 ==========

	// 월드 설정 메뉴. 블루프린트에서 수동 할당합니다.
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class UWorldSettingMenuUI* WorldSettingMenuUIWidget = nullptr;

	// 월드 설정 메뉴를 넣고 뒤로가기 델리게이트를 묶습니다.
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void SetWorldSettingMenuUI(class UWorldSettingMenuUI* InWorldSettingMenuUI);

	// 로드 메뉴. 블루프린트에서 수동 할당합니다.
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class ULoadMenuUI* LoadMenuUIWidget = nullptr;

	// 로드 메뉴를 넣고 뒤로가기 델리게이트를 묶습니다.
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void SetLoadMenuUI(class ULoadMenuUI* InLoadMenuUI);

	// 크레딧 메뉴. 블루프린트에서 수동 할당합니다.
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class UCreditUI* CreditMenuUIWidget = nullptr;

	// 크레딧 메뉴를 넣고 뒤로가기 델리게이트를 묶습니다.
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void SetCreditMenuUI(class UCreditUI* InCreditMenuUI);

	// 멀티플레이 메뉴. 블루프린트에서 수동 할당합니다.
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class UMultiplayerMenuUI* MultiplayerMenuUIWidget = nullptr;

	// 멀티플레이 메뉴를 넣고 뒤로가기 델리게이트를 묶습니다.
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void SetMultiplayerMenuUI(class UMultiplayerMenuUI* InMultiplayerMenuUI);

	// 모든 서브 메뉴를 숨기고 메인 캔버스를 다시 보여 줍니다.
	UFUNCTION()
	void ShowMainMenu();

	// 월드 설정 뒤로가기를 ShowMainMenu에 묶습니다.
	void BindWorldSettingMenuUIDelegate();

	// 로드 메뉴 뒤로가기를 ShowMainMenu에 묶습니다.
	void BindLoadMenuUIDelegate();

	// 크레딧 뒤로가기를 ShowMainMenu에 묶습니다.
	void BindCreditMenuUIDelegate();

	// 멀티플레이 뒤로가기를 ShowMainMenu에 묶습니다.
	void BindMultiplayerMenuUIDelegate();

	// 서브 메뉴에서 메인 메뉴로 돌아올 때 브로드캐스트합니다.
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnBackToMainMenu OnBackToMainMenu;
};
