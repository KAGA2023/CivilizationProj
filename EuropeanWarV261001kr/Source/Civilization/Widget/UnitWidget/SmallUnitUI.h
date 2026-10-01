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

	// HPBar 설정 (0.0 ~ 1.0)
	UFUNCTION(BlueprintCallable, Category = "Unit UI")
	void SetHPBar(float HealthPercent);

	// UnitImg 설정
	UFUNCTION(BlueprintCallable, Category = "Unit UI")
	void SetUnitImg(UTexture2D* UnitTexture);

	// CountryImg 설정
	UFUNCTION(BlueprintCallable, Category = "Unit UI")
	void SetCountryImg(UTexture2D* CountryTexture);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* HPBar;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* UnitImg;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg;
};
