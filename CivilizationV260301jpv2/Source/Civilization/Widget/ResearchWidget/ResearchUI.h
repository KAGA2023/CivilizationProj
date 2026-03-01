// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "ResearchUI.generated.h"

class UResearchComponent;

UCLASS()
class CIVILIZATION_API UResearchUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DevelopingTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* DevelopingImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* DevelopingBar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* OpenTechTreeBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTechTreeScrollUI* TechTreeScrollUIWidget = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* OpenTechTreeUI = nullptr;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* CloseTechTreeUI = nullptr;

	// 데이터 업데이트 함수
	UFUNCTION(BlueprintCallable, Category = "Research UI")
	void UpdateResearchData();

	// 연구 정보 업데이트 함수
	UFUNCTION(BlueprintCallable, Category = "Research UI")
	void UpdateResearchInfo();

	// 버튼 클릭 이벤트 핸들러
	UFUNCTION()
	void OnOpenTechTreeBtnClicked();

	// 기술 트리 Exit 버튼 클릭 핸들러 (CloseTechTreeUI 재생)
	UFUNCTION()
	void OnTechTreeExitClicked();

	// 연구 시작 델리게이트 핸들러
	UFUNCTION()
	void OnTechResearchStarted(FName TechID);

	// 연구 완료 델리게이트 핸들러
	UFUNCTION()
	void OnTechResearchCompleted(FName TechID);

	// 연구 진행도 변경 델리게이트 핸들러
	UFUNCTION()
	void OnTechResearchProgressChanged();

	// 시설 변경 델리게이트 핸들러
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// 연구 컴포넌트 참조
	UPROPERTY()
	UResearchComponent* CachedTechComponent = nullptr;

	// 델리게이트 바인딩
	void BindToResearchDelegates();
	void UnbindFromResearchDelegates();
	void BindToFacilityDelegates();
	void UnbindFromFacilityDelegates();
};

