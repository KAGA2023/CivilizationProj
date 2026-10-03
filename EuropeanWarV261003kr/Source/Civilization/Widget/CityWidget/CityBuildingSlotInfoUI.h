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
	// 건물 데이터로 이름·아이콘·산출 증가량을 채웁니다.
	UFUNCTION(BlueprintCallable, Category = "Building Slot Info")
	void SetupFromBuildingData(const FBuildingData& Data);

	// ========== BindWidget ==========

	// 건물 아이콘 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* IconImg = nullptr;

	// 건물 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* NameTxt = nullptr;

	// 건물 과학 증가량 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ScienceTxt = nullptr;

	// 건물 골드 증가량 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* GoldTxt = nullptr;

	// 건물 식량 증가량 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* FoodTxt = nullptr;

	// 건물 생산력 증가량 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ProductionTxt = nullptr;

	// 건물 체력 증가량 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* HealthTxt = nullptr;
};
