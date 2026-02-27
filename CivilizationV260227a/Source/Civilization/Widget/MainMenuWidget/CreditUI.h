// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreditUI.generated.h"

// 뒤로가기 버튼 클릭 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreditMenuBackButtonClicked);

UCLASS()
class CIVILIZATION_API UCreditUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 뒤로가기 버튼 클릭 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnCreditMenuBackButtonClicked OnBackButtonClickedDelegate;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn = nullptr;

	UFUNCTION()
	void OnBackButtonClicked();
};
