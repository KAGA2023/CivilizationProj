// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AIPlayerLoseUI.generated.h"

// 계속하기 버튼을 눌렀을 때 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAILoseContinueButtonClicked);

// AI 플레이어가 패배했을 때 보여주는 위젯입니다.
UCLASS()
class CIVILIZATION_API UAIPlayerLoseUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 패배한 AI의 국왕·국기로 UI를 채웁니다. MainHUD가 표시 전에 호출합니다.
	UFUNCTION(BlueprintCallable, Category = "AI Lose")
	void SetupForDefeatedPlayer(int32 DefeatedPlayerIndex);

	// 계속하기 버튼을 누르면 방송하는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnAILoseContinueButtonClicked OnContinueButtonClickedDelegate;

protected:
	// ========== BindWidget ==========

	// 패배한 AI 국왕 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* KingImg = nullptr;

	// 패배한 AI 국가 국기 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	// 패배 결과 문구 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ResultTxt = nullptr;

	// 게임을 이어가는 계속하기 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ContinueBtn;

private:
	// 계속하기 버튼을 누르면 델리게이트를 방송합니다.
	UFUNCTION()
	void OnContinueButtonClicked();
};
