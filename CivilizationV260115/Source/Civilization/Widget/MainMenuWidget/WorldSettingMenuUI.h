// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "WorldSettingMenuUI.generated.h"

// 뒤로가기 버튼 클릭 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWorldSettingMenuBackButtonClicked);

UCLASS()
class CIVILIZATION_API UWorldSettingMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 뒤로가기 버튼 클릭 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnWorldSettingMenuBackButtonClicked OnBackButtonClickedDelegate;

protected:
	// 시작 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* StartBtn = nullptr;

	// 뒤로가기 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn = nullptr;

	// 시작 버튼 클릭 핸들러
	UFUNCTION()
	void OnStartButtonClicked();

	// 뒤로가기 버튼 클릭 핸들러
	UFUNCTION()
	void OnBackButtonClicked();
};
