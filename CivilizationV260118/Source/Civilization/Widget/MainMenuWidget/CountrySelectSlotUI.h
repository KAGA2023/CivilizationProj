// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "CountrySelectSlotUI.generated.h"

UCLASS()
class CIVILIZATION_API UCountrySelectSlotUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// UI 요소 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Country Select Slot")
	void SetBorderColor(const FLinearColor& Color);

	UFUNCTION(BlueprintCallable, Category = "Country Select Slot")
	void SetCountryImage(class UTexture2D* Texture);

	UFUNCTION(BlueprintCallable, Category = "Country Select Slot")
	void SetCountryText(const FString& Text);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* ColorBrd = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* CountryTxt = nullptr;
};
