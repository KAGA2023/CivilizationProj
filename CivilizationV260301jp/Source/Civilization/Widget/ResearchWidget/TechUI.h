// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/VerticalBox.h"
#include "TechUI.generated.h"

class ASuperPlayerState;
class UCityComponent;
class UResearchComponent;
class UTechSlotUI;

UCLASS()
class CIVILIZATION_API UTechUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UTechUI(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeConstruct() override;

public:
	// 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Tech UI")
	void SetupTechUI(ASuperPlayerState* PlayerState);

	// 기술 VerticalBox
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* TechVB = nullptr;

private:
	// 델리게이트 바인딩
	void BindToResearchableTechsUpdated();
	void BindToFacilityDelegates();
	void UnbindFromFacilityDelegates();
	void BindToOwnedTilesChanged();
	void BindToProductionCompleted();

	// 연구 가능 목록 업데이트 시 호출
	UFUNCTION()
	void OnResearchableTechsUpdated(TArray<FName> ResearchableTechs);

	// 슬롯 클릭 핸들러
	UFUNCTION()
	void OnTechSlotClicked(FName TechRowName);

	// 시설 변경 델리게이트 핸들러
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);

	// 소유 타일 변경 시 호출 (타일 구매 → TurnTxt 갱신)
	UFUNCTION()
	void OnOwnedTilesChanged();

	// 생산 완료 시 호출 (건물 건설 → 과학량 변화로 TurnTxt 갱신)
	UFUNCTION()
	void OnProductionCompleted(FName ProductionID);

	// 슬롯 생성 및 추가
	void CreateTechSlots(const TArray<FName>& TechNames);

	// 기존 슬롯 제거
	void ClearAllSlots();

	// 모든 슬롯의 TurnTxt 갱신
	void UpdateAllSlotsTurnText();

	// 플레이어 상태 참조
	UPROPERTY()
	ASuperPlayerState* CachedPlayerState = nullptr;

	// 연구 컴포넌트 참조
	UPROPERTY()
	UResearchComponent* CachedTechComponent = nullptr;
};

