// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "CountrySlotUI.generated.h"

class ASuperPlayerState;

// 국가 슬롯 클릭 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountrySlotClicked, int32, TargetPlayerIndex);

UCLASS()
class CIVILIZATION_API UCountrySlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 플레이어 정보 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Country Slot")
	void SetupForPlayer(int32 PlayerIndex, ASuperPlayerState* PlayerState);

	// 대상 플레이어 인덱스 가져오기
	UFUNCTION(BlueprintCallable, Category = "Country Slot")
	int32 GetTargetPlayerIndex() const { return TargetPlayerIndex; }

	// 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Country Slot")
	FOnCountrySlotClicked OnCountrySlotClicked;

protected:
	virtual void NativeConstruct() override;

	// 버튼 클릭 핸들러
	UFUNCTION()
	void OnCountryBtnClicked();

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CountryBtn = nullptr;

private:
	// 대상 플레이어 인덱스
	UPROPERTY()
	int32 TargetPlayerIndex = -1;
};

