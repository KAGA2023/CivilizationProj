// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ResearchUI.generated.h"

class UTechComponent;

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

	// 연구 정보 업데이트 함수
	UFUNCTION(BlueprintCallable, Category = "Research UI")
	void UpdateResearchInfo();

	// 연구 시작 델리게이트 핸들러
	UFUNCTION()
	void OnTechResearchStarted(FName TechRowName);

	// 연구 완료 델리게이트 핸들러
	UFUNCTION()
	void OnTechResearchCompleted(FName TechRowName);

	// 연구 진행도 변경 델리게이트 핸들러
	UFUNCTION()
	void OnTechResearchProgressChanged();

	// 버튼 클릭 이벤트 핸들러
	UFUNCTION()
	void OnOpenTechTreeBtnClicked();

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// 기술 컴포넌트 참조
	UPROPERTY()
	UTechComponent* CachedTechComponent = nullptr;

	// 델리게이트 바인딩
	void BindToResearchDelegates();
	void UnbindFromResearchDelegates();
};

