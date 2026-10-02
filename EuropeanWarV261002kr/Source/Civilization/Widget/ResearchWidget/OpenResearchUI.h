// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OpenResearchUI.generated.h"

class UResearchComponent;

// 연구 열기 버튼을 눌렀을 때 브로드캐스트합니다. MainHUD가 애니메이션 재생에 씁니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnOpenResearchButtonClicked);

UCLASS()
class CIVILIZATION_API UOpenResearchUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 연구 열기 버튼을 누르면 방송하는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Research Events")
	FOnOpenResearchButtonClicked OnOpenResearchButtonClicked;

	// ========== BindWidget ==========

	// 연구 UI를 여는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* OpenResearchBtn = nullptr;

	// 개발 중인 기술 아이콘
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* DevelopingImg = nullptr;

	// 개발 중인 기술 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DevelopingTxt = nullptr;

	// 개발 중인 기술 진행 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* DevelopingBar = nullptr;

	// 개발 중인 기술 이름·아이콘·진행 바를 갱신합니다. ResearchUI와 같은 로직입니다.
	UFUNCTION(BlueprintCallable, Category = "Research UI")
	void UpdateResearchInfo();

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// ========== 핸들러 ==========

	// 연구 열기 버튼을 누르면 델리게이트를 방송합니다.
	UFUNCTION()
	void OnOpenResearchBtnClicked();

	// 기술 연구가 시작되면 진행 표시를 갱신합니다.
	UFUNCTION()
	void OnTechResearchStarted(FName TechID);

	// 기술 연구가 끝나면 진행 표시를 갱신합니다.
	UFUNCTION()
	void OnTechResearchCompleted(FName TechID);

	// 연구 진행도가 바뀌면 진행 바를 갱신합니다.
	UFUNCTION()
	void OnTechResearchProgressChanged();

	// 연구 델리게이트를 해제합니다.
	void UnbindFromResearchDelegates();

	// 이 UI가 가리키는 연구 컴포넌트
	UPROPERTY()
	class UResearchComponent* CachedTechComponent = nullptr;
};
