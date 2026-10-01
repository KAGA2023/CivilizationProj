// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CityBuildingSlotInfoUI.generated.h"

struct FBuildingData;

UCLASS()
class CIVILIZATION_API UCityBuildingSlotInfoUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 건물 생산량(증가량)으로 UI 설정
	UFUNCTION(BlueprintCallable, Category = "Building Slot Info")
	void SetupFromBuildingData(const FBuildingData& Data);

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* IconImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* NameTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ScienceTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* GoldTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* FoodTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ProductionTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* HealthTxt = nullptr;
};
