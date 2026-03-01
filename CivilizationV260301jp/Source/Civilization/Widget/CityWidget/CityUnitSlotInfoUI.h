// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CityUnitSlotInfoUI.generated.h"

struct FUnitBaseStat;

UCLASS()
class CIVILIZATION_API UCityUnitSlotInfoUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 유닛 베이스 스탯으로 UI 설정
	UFUNCTION(BlueprintCallable, Category = "Unit Slot Info")
	void SetupFromUnitData(const FUnitBaseStat& Data);

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* IconImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* NameTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* AttackTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DefenseAttackTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* HealthTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* MovementTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* RangeTxt = nullptr;
};
