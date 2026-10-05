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
	// 도시 체력 바를 0.0~1.0 비율로 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "City UI")
	void SetHPBar(float HealthPercent);

	// 도시 소유 국가 이름 텍스트를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "City UI")
	void SetCountryNameTxt(const FString& CountryName);

	// 도시 소유 국가 국기 이미지를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "City UI")
	void SetCountryImg(UTexture2D* CountryTexture);

protected:
	// ========== BindWidget ==========

	// 도시 소유 국가 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* CountryNameTxt = nullptr;

	// 도시 소유 국가 국기 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	// 도시 체력 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* HPBar = nullptr;
};
