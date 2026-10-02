// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UnitInfoUI.generated.h"

class AUnitCharacterBase;

UCLASS()
class CIVILIZATION_API UUnitInfoUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== BindWidget ==========

	// 선택 유닛 체력 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* HpBar = nullptr;

	// 선택 유닛을 해산하는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* DeathBtn = nullptr;

	// 선택 유닛 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitNameTxt = nullptr;

	// 선택 유닛 공격력 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitAttackStrengthTxt = nullptr;

	// 선택 유닛 방어력 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitDefenceStrengthTxt = nullptr;

	// 선택 유닛 체력 숫자 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitHealthTxt = nullptr;

	// 선택 유닛 사거리 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitRangeTxt = nullptr;

	// 선택 유닛 이동력 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitMovementPointTxt = nullptr;

	// 현재 선택된 아군 유닛 정보로 UI를 채웁니다. MainHUD가 호출합니다.
	UFUNCTION(BlueprintCallable, Category = "Unit Info")
	void SetupForUnit(AUnitCharacterBase* Unit);

protected:
	virtual void NativeConstruct() override;

private:
	// 해산 버튼이 가리키는 선택 유닛
	UPROPERTY()
	TWeakObjectPtr<AUnitCharacterBase> CachedUnit;

	// 해산 버튼을 누르면 캐시된 유닛을 제거합니다.
	UFUNCTION()
	void OnDeathBtnClicked();
};
