// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OpenResearchUI.generated.h"

class UResearchComponent;

// OpenResearchBtn 클릭 델리게이트 (MainHUD에서 애니메이션 재생 등에 사용)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOpenResearchButtonClicked);

UCLASS()
class CIVILIZATION_API UOpenResearchUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Research Events")
	FOnOpenResearchButtonClicked OnOpenResearchButtonClicked;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* OpenResearchBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* DevelopingImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DevelopingTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* DevelopingBar = nullptr;

	// 개발 중인 기술 표시 갱신 (ResearchUI와 동일 로직)
	UFUNCTION(BlueprintCallable, Category = "Research UI")
	void UpdateResearchInfo();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UFUNCTION()
	void OnOpenResearchBtnClicked();

	UFUNCTION()
	void OnTechResearchStarted(FName TechID);
	UFUNCTION()
	void OnTechResearchCompleted(FName TechID);
	UFUNCTION()
	void OnTechResearchProgressChanged();

	void UnbindFromResearchDelegates();

	UPROPERTY()
	class UResearchComponent* CachedTechComponent = nullptr;
};
