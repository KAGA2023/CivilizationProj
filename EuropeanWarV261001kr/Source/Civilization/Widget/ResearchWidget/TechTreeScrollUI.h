// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../../Research/Research.h"
#include "TechTreeScrollUI.generated.h"

class UResearchComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTechTreeExitClicked);

UCLASS()
class CIVILIZATION_API UTechTreeScrollUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ExitBtn 클릭 시 브로드캐스트 (ResearchUI에서 CloseTechTreeUI 재생용)
	UPROPERTY(BlueprintAssignable, Category = "Tech Tree")
	FOnTechTreeExitClicked OnExitClicked;

	// 연구 완료 표시 Img 갱신 (플레이어 0 기준)
	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	void UpdateTechCompleteIndicators();

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ExitBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* InfoTechIconImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* InfoTechNameTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TechInfoTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TechDescriptionTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TechCompleteTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TechNotCompleteTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PotteryBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* PotteryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* AnimalHusbandryBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* AnimalHusbandryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MiningBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MiningImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* IrrigationBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* IrrigationImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* WritingBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* WritingImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ArcheryBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ArcheryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MasonryBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MasonryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BronzeWorkingBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* BronzeWorkingImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* WheelBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* WheelImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MathematicsBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MathematicsImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CurrencyBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CurrencyImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MilitaryTacticsBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MilitaryTacticsImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ConstructionBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ConstructionImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* IronWorkingBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* IronWorkingImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* EngineeringBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* EngineeringImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* EducationBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* EducationImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ApprenticeshipBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ApprenticeshipImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* StirrupsBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* StirrupsImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MilitaryEngineeringBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MilitaryEngineeringImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CastlesBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CastlesImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MachineryBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MachineryImg = nullptr;

	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnExitBtnClicked();

	UFUNCTION()
	void OnPotteryBtnHovered();

	UFUNCTION()
	void OnPotteryBtnUnhovered();

	UFUNCTION()
	void OnAnimalHusbandryBtnHovered();

	UFUNCTION()
	void OnAnimalHusbandryBtnUnhovered();

	UFUNCTION()
	void OnMiningBtnHovered();

	UFUNCTION()
	void OnMiningBtnUnhovered();

	UFUNCTION()
	void OnIrrigationBtnHovered();

	UFUNCTION()
	void OnIrrigationBtnUnhovered();

	UFUNCTION()
	void OnWritingBtnHovered();

	UFUNCTION()
	void OnWritingBtnUnhovered();

	UFUNCTION()
	void OnArcheryBtnHovered();

	UFUNCTION()
	void OnArcheryBtnUnhovered();

	UFUNCTION()
	void OnMasonryBtnHovered();

	UFUNCTION()
	void OnMasonryBtnUnhovered();

	UFUNCTION()
	void OnBronzeWorkingBtnHovered();

	UFUNCTION()
	void OnBronzeWorkingBtnUnhovered();

	UFUNCTION()
	void OnWheelBtnHovered();

	UFUNCTION()
	void OnWheelBtnUnhovered();

	UFUNCTION()
	void OnMathematicsBtnHovered();

	UFUNCTION()
	void OnMathematicsBtnUnhovered();

	UFUNCTION()
	void OnCurrencyBtnHovered();

	UFUNCTION()
	void OnCurrencyBtnUnhovered();

	UFUNCTION()
	void OnMilitaryTacticsBtnHovered();

	UFUNCTION()
	void OnMilitaryTacticsBtnUnhovered();

	UFUNCTION()
	void OnConstructionBtnHovered();

	UFUNCTION()
	void OnConstructionBtnUnhovered();

	UFUNCTION()
	void OnIronWorkingBtnHovered();

	UFUNCTION()
	void OnIronWorkingBtnUnhovered();

	UFUNCTION()
	void OnEngineeringBtnHovered();

	UFUNCTION()
	void OnEngineeringBtnUnhovered();

	UFUNCTION()
	void OnEducationBtnHovered();

	UFUNCTION()
	void OnEducationBtnUnhovered();

	UFUNCTION()
	void OnApprenticeshipBtnHovered();

	UFUNCTION()
	void OnApprenticeshipBtnUnhovered();

	UFUNCTION()
	void OnStirrupsBtnHovered();

	UFUNCTION()
	void OnStirrupsBtnUnhovered();

	UFUNCTION()
	void OnMilitaryEngineeringBtnHovered();

	UFUNCTION()
	void OnMilitaryEngineeringBtnUnhovered();

	UFUNCTION()
	void OnCastlesBtnHovered();

	UFUNCTION()
	void OnCastlesBtnUnhovered();

	UFUNCTION()
	void OnMachineryBtnHovered();

	UFUNCTION()
	void OnMachineryBtnUnhovered();

private:
	void SetInfoFromTech(FName TechRowName);
	void ClearInfoPanel();
	FString BuildUnlockInfoString(const FTechData& TechData) const;
};
