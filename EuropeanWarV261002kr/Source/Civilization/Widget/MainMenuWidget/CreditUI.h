// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreditUI.generated.h"

// 크레딧을 닫고 메인 메뉴로 돌아갈 때 씁니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCreditMenuBackButtonClicked);

UCLASS()
class CIVILIZATION_API UCreditUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 수명 주기 ==========

	// 뒤로가기 버튼을 묶습니다.
	virtual void NativeConstruct() override;

	// 메인 메뉴가 크레딧을 닫을 때 듣는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnCreditMenuBackButtonClicked OnBackButtonClickedDelegate;

protected:
	// ========== BindWidget ==========

	// 크레딧을 닫고 메인 메뉴로 돌아가는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn = nullptr;

	// 뒤로가기 델리게이트를 브로드캐스트합니다.
	UFUNCTION()
	void OnBackButtonClicked();
};
