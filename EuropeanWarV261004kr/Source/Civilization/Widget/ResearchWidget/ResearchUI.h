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
	// ========== BindWidget ==========

	// 개발 중인 기술 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* DevelopingTxt = nullptr;

	// 개발 중인 기술 아이콘
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* DevelopingImg = nullptr;

	// 개발 중인 기술 진행 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* DevelopingBar = nullptr;

	// 기술 트리를 여는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* OpenTechTreeBtn = nullptr;

	// 기술 트리 스크롤 위젯
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTechTreeScrollUI* TechTreeScrollUIWidget = nullptr;

	// 기술 트리를 열 때 재생하는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* OpenTechTreeUI = nullptr;

	// 기술 트리를 닫을 때 재생하는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* CloseTechTreeUI = nullptr;

	// ========== 갱신 ==========

	// 연구 UI 전체를 현재 상태로 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "Research UI")
	void UpdateResearchData();

	// 개발 중인 기술 이름·아이콘·진행 바를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "Research UI")
	void UpdateResearchInfo();

	// ========== 버튼 / 델리게이트 ==========

	// 기술 트리 버튼을 누르면 열기 애니메이션을 재생합니다.
	UFUNCTION()
	void OnOpenTechTreeBtnClicked();

	// 기술 트리 종료 버튼을 누르면 닫기 애니메이션을 재생합니다.
	UFUNCTION()
	void OnTechTreeExitClicked();

	// 기술 연구가 시작되면 진행 표시를 갱신합니다.
	UFUNCTION()
	void OnTechResearchStarted(FName TechID);

	// 기술 연구가 끝나면 진행 표시를 갱신합니다.
	UFUNCTION()
	void OnTechResearchCompleted(FName TechID);

	// 연구 진행도가 바뀌면 진행 바를 갱신합니다.
	UFUNCTION()
	void OnTechResearchProgressChanged();

	// 시설이 바뀌면 연구 정보를 다시 그립니다.
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// ========== 캐시 / 바인딩 ==========

	// 이 UI가 가리키는 연구 컴포넌트
	UPROPERTY()
	UResearchComponent* CachedTechComponent = nullptr;

	// 연구 델리게이트를 연결합니다.
	void BindToResearchDelegates();

	// 연구 델리게이트를 해제합니다.
	void UnbindFromResearchDelegates();

	// 시설 델리게이트를 연결합니다.
	void BindToFacilityDelegates();

	// 시설 델리게이트를 해제합니다.
	void UnbindFromFacilityDelegates();
};
