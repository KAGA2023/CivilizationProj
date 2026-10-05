// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "CountrySlotUI.generated.h"

class ASuperPlayerState;

// 국가 슬롯을 클릭했을 때 대상 플레이어 인덱스를 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountrySlotClicked, int32, TargetPlayerIndex);

UCLASS()
class CIVILIZATION_API UCountrySlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 대상 플레이어의 국기와 인덱스로 슬롯을 채웁니다.
	UFUNCTION(BlueprintCallable, Category = "Country Slot")
	void SetupForPlayer(int32 PlayerIndex, ASuperPlayerState* PlayerState);

	// 이 슬롯이 가리키는 대상 플레이어 인덱스를 반환합니다.
	UFUNCTION(BlueprintCallable, Category = "Country Slot")
	int32 GetTargetPlayerIndex() const { return TargetPlayerIndex; }

	// 국가 슬롯 클릭을 알리는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Country Slot")
	FOnCountrySlotClicked OnCountrySlotClicked;

protected:
	virtual void NativeConstruct() override;

	// 국가 버튼을 누르면 클릭 델리게이트를 방송합니다.
	UFUNCTION()
	void OnCountryBtnClicked();

	// ========== BindWidget ==========

	// 국가 국기 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	// 국가 슬롯을 누르는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CountryBtn = nullptr;

private:
	// 이 슬롯이 가리키는 대상 플레이어 인덱스
	UPROPERTY()
	int32 TargetPlayerIndex = -1;
};
