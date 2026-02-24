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
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* HpBar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* DeathBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitNameTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitAttackStrengthTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitDefenceStrengthTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitHealthTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitRangeTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* UnitMovementPointTxt = nullptr;

	// 현재 선택된 아군 유닛 정보로 UI 갱신 (MainHUD에서 호출)
	UFUNCTION(BlueprintCallable, Category = "Unit Info")
	void SetupForUnit(AUnitCharacterBase* Unit);

protected:
	virtual void NativeConstruct() override;

private:
	UPROPERTY()
	TWeakObjectPtr<AUnitCharacterBase> CachedUnit;

	UFUNCTION()
	void OnDeathBtnClicked();
};
