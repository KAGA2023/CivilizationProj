// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SmallCityUI.generated.h"

UCLASS()
class CIVILIZATION_API USmallCityUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// HPBar 설정 (0.0 ~ 1.0)
	UFUNCTION(BlueprintCallable, Category = "City UI")
	void SetHPBar(float HealthPercent);

	// CountryNameTxt 설정
	UFUNCTION(BlueprintCallable, Category = "City UI")
	void SetCountryNameTxt(const FString& CountryName);

	// CountryImg 설정
	UFUNCTION(BlueprintCallable, Category = "City UI")
	void SetCountryImg(UTexture2D* CountryTexture);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* CountryNameTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* HPBar = nullptr;
};
