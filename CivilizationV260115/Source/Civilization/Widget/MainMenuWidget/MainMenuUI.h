// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "MainMenuUI.generated.h"

class UWorldSettingMenuUI;

// 메인 메뉴로 돌아가기 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackToMainMenu);

/**
 * 메인 메뉴 UI 위젯
 */
UCLASS()
class CIVILIZATION_API UMainMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// 메인 메뉴 캔버스 패널
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UCanvasPanel* MainMenuCanvas = nullptr;

	// 플레이 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PlayBtn = nullptr;

	// 종료 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* EndBtn = nullptr;

	// 로드 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* LoadBtn = nullptr;

	// 플레이 버튼 클릭 핸들러
	UFUNCTION()
	void OnPlayButtonClicked();

	// 종료 버튼 클릭 핸들러
	UFUNCTION()
	void OnEndButtonClicked();

	// 로드 버튼 클릭 핸들러
	UFUNCTION()
	void OnLoadButtonClicked();

	// WorldSettingMenuUI 위젯 참조 (블루프린트에서 수동 할당)
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class UWorldSettingMenuUI* WorldSettingMenuUIWidget = nullptr;

	// WorldSettingMenuUI 위젯 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void SetWorldSettingMenuUI(class UWorldSettingMenuUI* InWorldSettingMenuUI);

	// LoadMenuUI 위젯 참조 (블루프린트에서 수동 할당)
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class ULoadMenuUI* LoadMenuUIWidget = nullptr;

	// LoadMenuUI 위젯 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void SetLoadMenuUI(class ULoadMenuUI* InLoadMenuUI);

	// 메인 메뉴로 돌아가기 함수
	UFUNCTION()
	void ShowMainMenu();

	// WorldSettingMenuUI 델리게이트 바인딩 함수
	void BindWorldSettingMenuUIDelegate();

	// LoadMenuUI 델리게이트 바인딩 함수
	void BindLoadMenuUIDelegate();

	// 메인 메뉴로 돌아가기 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnBackToMainMenu OnBackToMainMenu;
};
