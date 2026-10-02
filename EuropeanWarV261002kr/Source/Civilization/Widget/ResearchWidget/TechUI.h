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
	// 로컬 플레이어의 연구 가능 기술로 목록을 채웁니다.
	UFUNCTION(BlueprintCallable, Category = "Tech UI")
	void SetupTechUI(ASuperPlayerState* PlayerState);

	// 연구 가능 기술 슬롯을 담는 세로 박스
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* TechVB = nullptr;

private:
	// ========== 바인딩 / 핸들러 ==========

	// 연구 가능 목록 갱신 델리게이트를 연결합니다.
	void BindToResearchableTechsUpdated();

	// 시설 변경 델리게이트를 연결합니다.
	void BindToFacilityDelegates();

	// 시설 변경 델리게이트를 해제합니다.
	void UnbindFromFacilityDelegates();

	// 소유 타일 변경 델리게이트를 연결합니다.
	void BindToOwnedTilesChanged();

	// 생산 완료 델리게이트를 연결합니다.
	void BindToProductionCompleted();

	// 연구 가능 목록이 바뀌면 슬롯을 다시 만듭니다.
	UFUNCTION()
	void OnResearchableTechsUpdated(TArray<FName> ResearchableTechs);

	// 기술 슬롯을 클릭하면 해당 기술 연구를 시작합니다.
	UFUNCTION()
	void OnTechSlotClicked(FName TechRowName);

	// 시설이 바뀌면 슬롯 턴 수를 다시 계산합니다.
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);

	// 소유 타일이 바뀌면 슬롯 턴 수를 갱신합니다.
	UFUNCTION()
	void OnOwnedTilesChanged();

	// 건물 생산이 끝나면 과학량 변화로 슬롯 턴 수를 갱신합니다.
	UFUNCTION()
	void OnProductionCompleted(FName ProductionID);

	// ========== 슬롯 ==========

	// 기술 슬롯을 만들어 세로 박스에 넣습니다.
	void CreateTechSlots(const TArray<FName>& TechNames);

	// 기존 기술 슬롯을 모두 지웁니다.
	void ClearAllSlots();

	// 모든 슬롯의 남은 턴 텍스트를 갱신합니다.
	void UpdateAllSlotsTurnText();

	// ========== 캐시 ==========

	// 기술 UI가 가리키는 플레이어 스테이트
	UPROPERTY()
	ASuperPlayerState* CachedPlayerState = nullptr;

	// 기술 UI가 가리키는 연구 컴포넌트
	UPROPERTY()
	UResearchComponent* CachedTechComponent = nullptr;
};
