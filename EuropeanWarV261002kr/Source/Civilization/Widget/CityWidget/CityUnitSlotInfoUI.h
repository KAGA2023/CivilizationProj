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
	// 유닛 베이스 스탯으로 이름·아이콘·전투 수치를 채웁니다.
	UFUNCTION(BlueprintCallable, Category = "Unit Slot Info")
	void SetupFromUnitData(const FUnitBaseStat& Data);

	// ========== BindWidget ==========

	// 유닛 아이콘 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UImage* IconImg = nullptr;

	// 유닛 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* NameTxt = nullptr;

	// 유닛 공격력 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* AttackTxt = nullptr;

	// 유닛 방어 공격력 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DefenseAttackTxt = nullptr;

	// 유닛 체력 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* HealthTxt = nullptr;

	// 유닛 이동력 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* MovementTxt = nullptr;

	// 유닛 사거리 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* RangeTxt = nullptr;
};
