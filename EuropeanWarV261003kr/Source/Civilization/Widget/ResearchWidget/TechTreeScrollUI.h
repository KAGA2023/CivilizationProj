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
	// Exit 버튼을 누르면 방송합니다. ResearchUI가 닫기 애니메이션을 재생합니다.
	UPROPERTY(BlueprintAssignable, Category = "Tech Tree")
	FOnTechTreeExitClicked OnExitClicked;

	// 플레이어 0 기준으로 연구 완료 표시 이미지를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "Tech Tree")
	void UpdateTechCompleteIndicators();

protected:
	// ========== 정보 패널 ==========

	// 기술 트리를 닫는 종료 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ExitBtn = nullptr;

	// 호버한 기술 아이콘
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* InfoTechIconImg = nullptr;

	// 호버한 기술 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* InfoTechNameTxt = nullptr;

	// 호버한 기술이 해제하는 항목 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TechInfoTxt = nullptr;

	// 호버한 기술 설명 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TechDescriptionTxt = nullptr;

	// 연구 완료일 때 켜는 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TechCompleteTxt = nullptr;

	// 연구 미완료일 때 켜는 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* TechNotCompleteTxt = nullptr;

	// ========== 기술 트리 버튼 / 완료 표시 ==========

	// 토기 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PotteryBtn = nullptr;

	// 토기 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* PotteryImg = nullptr;

	// 목축 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* AnimalHusbandryBtn = nullptr;

	// 목축 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* AnimalHusbandryImg = nullptr;

	// 채광 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MiningBtn = nullptr;

	// 채광 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MiningImg = nullptr;

	// 관개 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* IrrigationBtn = nullptr;

	// 관개 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* IrrigationImg = nullptr;

	// 문자 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* WritingBtn = nullptr;

	// 문자 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* WritingImg = nullptr;

	// 활쏘기 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ArcheryBtn = nullptr;

	// 활쏘기 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ArcheryImg = nullptr;

	// 석공 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MasonryBtn = nullptr;

	// 석공 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MasonryImg = nullptr;

	// 청동 가공 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BronzeWorkingBtn = nullptr;

	// 청동 가공 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* BronzeWorkingImg = nullptr;

	// 바퀴 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* WheelBtn = nullptr;

	// 바퀴 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* WheelImg = nullptr;

	// 수학 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MathematicsBtn = nullptr;

	// 수학 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MathematicsImg = nullptr;

	// 화폐 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CurrencyBtn = nullptr;

	// 화폐 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CurrencyImg = nullptr;

	// 전술 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MilitaryTacticsBtn = nullptr;

	// 전술 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MilitaryTacticsImg = nullptr;

	// 건축 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ConstructionBtn = nullptr;

	// 건축 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ConstructionImg = nullptr;

	// 철기 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* IronWorkingBtn = nullptr;

	// 철기 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* IronWorkingImg = nullptr;

	// 공학 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* EngineeringBtn = nullptr;

	// 공학 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* EngineeringImg = nullptr;

	// 교육 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* EducationBtn = nullptr;

	// 교육 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* EducationImg = nullptr;

	// 도제 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ApprenticeshipBtn = nullptr;

	// 도제 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ApprenticeshipImg = nullptr;

	// 등자 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* StirrupsBtn = nullptr;

	// 등자 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* StirrupsImg = nullptr;

	// 군사 공학 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MilitaryEngineeringBtn = nullptr;

	// 군사 공학 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MilitaryEngineeringImg = nullptr;

	// 성 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CastlesBtn = nullptr;

	// 성 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CastlesImg = nullptr;

	// 기계 기술 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* MachineryBtn = nullptr;

	// 기계 연구 완료 표시 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* MachineryImg = nullptr;

	virtual void NativeConstruct() override;

	// ========== 호버 핸들러 ==========

	// 종료 버튼을 누르면 Exit 델리게이트를 방송합니다.
	UFUNCTION()
	void OnExitBtnClicked();

	// 토기 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnPotteryBtnHovered();

	// 토기 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnPotteryBtnUnhovered();

	// 목축 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnAnimalHusbandryBtnHovered();

	// 목축 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnAnimalHusbandryBtnUnhovered();

	// 채광 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnMiningBtnHovered();

	// 채광 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnMiningBtnUnhovered();

	// 관개 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnIrrigationBtnHovered();

	// 관개 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnIrrigationBtnUnhovered();

	// 문자 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnWritingBtnHovered();

	// 문자 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnWritingBtnUnhovered();

	// 활쏘기 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnArcheryBtnHovered();

	// 활쏘기 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnArcheryBtnUnhovered();

	// 석공 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnMasonryBtnHovered();

	// 석공 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnMasonryBtnUnhovered();

	// 청동 가공 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnBronzeWorkingBtnHovered();

	// 청동 가공 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnBronzeWorkingBtnUnhovered();

	// 바퀴 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnWheelBtnHovered();

	// 바퀴 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnWheelBtnUnhovered();

	// 수학 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnMathematicsBtnHovered();

	// 수학 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnMathematicsBtnUnhovered();

	// 화폐 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnCurrencyBtnHovered();

	// 화폐 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnCurrencyBtnUnhovered();

	// 전술 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnMilitaryTacticsBtnHovered();

	// 전술 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnMilitaryTacticsBtnUnhovered();

	// 건축 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnConstructionBtnHovered();

	// 건축 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnConstructionBtnUnhovered();

	// 철기 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnIronWorkingBtnHovered();

	// 철기 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnIronWorkingBtnUnhovered();

	// 공학 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnEngineeringBtnHovered();

	// 공학 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnEngineeringBtnUnhovered();

	// 교육 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnEducationBtnHovered();

	// 교육 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnEducationBtnUnhovered();

	// 도제 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnApprenticeshipBtnHovered();

	// 도제 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnApprenticeshipBtnUnhovered();

	// 등자 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnStirrupsBtnHovered();

	// 등자 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnStirrupsBtnUnhovered();

	// 군사 공학 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnMilitaryEngineeringBtnHovered();

	// 군사 공학 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnMilitaryEngineeringBtnUnhovered();

	// 성 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnCastlesBtnHovered();

	// 성 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnCastlesBtnUnhovered();

	// 기계 버튼을 호버하면 정보 패널을 채웁니다.
	UFUNCTION()
	void OnMachineryBtnHovered();

	// 기계 버튼 호버가 끝나면 정보 패널을 비웁니다.
	UFUNCTION()
	void OnMachineryBtnUnhovered();

private:
	// 기술 RowName으로 정보 패널을 채웁니다.
	void SetInfoFromTech(FName TechRowName);

	// 정보 패널 텍스트와 아이콘을 비웁니다.
	void ClearInfoPanel();

	// 기술이 해제하는 시설·건물·유닛을 문자열로 만듭니다.
	FString BuildUnlockInfoString(const FTechData& TechData) const;
};
