// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SmallUnitUI.generated.h"

UCLASS()
class CIVILIZATION_API USmallUnitUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	// 유닛 체력 바를 0.0~1.0 비율로 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Unit UI")
	void SetHPBar(float HealthPercent);

	// 유닛 아이콘 이미지를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Unit UI")
	void SetUnitImg(UTexture2D* UnitTexture);

	// 유닛 소유 국가 국기 이미지를 설정합니다.
	UFUNCTION(BlueprintCallable, Category = "Unit UI")
	void SetCountryImg(UTexture2D* CountryTexture);

protected:
	// ========== BindWidget ==========

	// 유닛 체력 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* HPBar;

	// 유닛 아이콘 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* UnitImg;

	// 유닛 소유 국가 국기 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg;
};
