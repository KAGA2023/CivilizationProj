// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CityUI.generated.h"

class UCityComponent;

UCLASS()
class CIVILIZATION_API UCityUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ========== BindWidget ==========

	// 도시 과학 산출 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ScienceTxt = nullptr;

	// 도시 골드 산출 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* GoldTxt = nullptr;

	// 도시 생산력 산출 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ProductionTxt = nullptr;

	// 도시 식량 산출 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* FoodTxt = nullptr;

	// 지금 생산 중인 항목 이름 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ProducingTxt = nullptr;

	// 지금 생산 중인 항목 아이콘
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ProducingImg = nullptr;

	// 지금 생산 중인 항목 진행 바
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* ProducingBar = nullptr;

	// 생산 목록 창을 여는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ProductionBtn = nullptr;

	// 즉시 구매 목록 창을 여는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PurchaseBtn = nullptr;

	// 타일 구매 모드를 켜는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PurchaseTileBtn = nullptr;

	// 타일 구매 모드를 끄는 닫기 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UButton* CloseBtn = nullptr;

	// 생산 목록 위젯을 감싸는 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* ProductionWid = nullptr;

	// 즉시 구매 목록 위젯을 감싸는 보더
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* PurchaseWid = nullptr;

	// ========== 갱신 ==========

	// 도시 산출량 텍스트를 현재 값으로 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "City UI")
	void UpdateCityData();

	// 생산 중 항목 이름·아이콘·진행 바를 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "City UI")
	void UpdateProductionInfo();

	// ========== 버튼 / 델리게이트 ==========

	// 생산 버튼을 누르면 생산 목록을 엽니다.
	UFUNCTION()
	void OnProductionBtnClicked();

	// 구매 버튼을 누르면 즉시 구매 목록을 엽니다.
	UFUNCTION()
	void OnPurchaseBtnClicked();

	// 생산이 시작되면 생산 정보를 갱신합니다.
	UFUNCTION()
	void OnProductionStarted(FName ProductionID);

	// 생산이 끝나면 생산 정보를 갱신합니다.
	UFUNCTION()
	void OnProductionCompleted(FName ProductionID);

	// 생산 진행도가 바뀌면 진행 바를 갱신합니다.
	UFUNCTION()
	void OnProductionProgressChanged();

	// 시설이 바뀌면 도시 산출량을 다시 그립니다.
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);

	// 타일 구매 버튼을 누르면 구매 가능 타일을 강조합니다.
	UFUNCTION()
	void OnPurchaseTileBtnClicked();

	// 닫기 버튼을 누르면 타일 구매 모드를 해제합니다.
	UFUNCTION()
	void OnCloseBtnClicked();

	// 구매 가능 타일을 클릭하면 그 타일을 삽니다.
	UFUNCTION()
	void OnPurchaseTileClickedHandler(FVector2D TileCoordinate);

	// 골드가 바뀌면 도시 골드 텍스트를 갱신합니다.
	UFUNCTION()
	void OnGoldChanged(int32 NewGold);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// ========== 캐시 / 바인딩 ==========

	// 이 UI가 가리키는 도시 컴포넌트
	UPROPERTY()
	UCityComponent* CachedCityComponent = nullptr;

	// 생산 델리게이트를 연결합니다.
	void BindToProductionDelegates();

	// 생산 델리게이트를 해제합니다.
	void UnbindFromProductionDelegates();

	// 시설 델리게이트를 연결합니다.
	void BindToFacilityDelegates();

	// 시설 델리게이트를 해제합니다.
	void UnbindFromFacilityDelegates();

	// 타일 구매 델리게이트를 연결합니다.
	void BindToPurchaseDelegates();

	// 타일 구매 델리게이트를 해제합니다.
	void UnbindFromPurchaseDelegates();

	// ========== 타일 구매 ==========

	// 지금 살 수 있는 타일 좌표를 찾습니다.
	void FindPurchaseableTiles();

	// 살 수 있는 타일에 구매 UI를 켭니다.
	void HighlightPurchaseableTiles();

	// 구매 가능 타일 강조를 지웁니다.
	void ClearPurchaseableTileHighlights();

	// 타일 구매 모드를 끄고 강조를 해제합니다.
	void ExitPurchaseMode();

	// 지금 타일 구매 모드인지
	UPROPERTY()
	bool bIsTilePurchaseMode = false;

	// 지금 살 수 있는 타일 좌표 목록
	UPROPERTY()
	TArray<FVector2D> PurchaseableTileCoordinates;
};
