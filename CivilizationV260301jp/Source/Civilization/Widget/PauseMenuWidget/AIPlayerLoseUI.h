// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "AIPlayerLoseUI.generated.h"

// 계속하기 버튼 클릭 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAILoseContinueButtonClicked);

/**
 * AI 플레이어 패배 위젯
 */
UCLASS()
class CIVILIZATION_API UAIPlayerLoseUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 패배한 AI 플레이어 정보로 UI 세팅 (MainHUD에서 표시 전에 호출)
	UFUNCTION(BlueprintCallable, Category = "AI Lose")
	void SetupForDefeatedPlayer(int32 DefeatedPlayerIndex);

	// 계속하기 버튼 클릭 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnAILoseContinueButtonClicked OnContinueButtonClickedDelegate;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* KingImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ResultTxt = nullptr;

	// 계속하기 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ContinueBtn;

private:
	// 계속하기 버튼 클릭 이벤트
	UFUNCTION()
	void OnContinueButtonClicked();
};
