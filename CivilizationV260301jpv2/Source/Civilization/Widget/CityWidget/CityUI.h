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
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ScienceTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* GoldTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ProductionTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* FoodTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ProducingTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* ProducingImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UProgressBar* ProducingBar = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* ProductionBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PurchaseBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PurchaseTileBtn = nullptr;

	// 타일 구매 모드 닫기 버튼 (클릭 시 구매 모드 해제, 위젯에서 바인드)
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UButton* CloseBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* ProductionWid = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* PurchaseWid = nullptr;

	// 데이터 업데이트 함수
	UFUNCTION(BlueprintCallable, Category = "City UI")
	void UpdateCityData();

	// 생산 정보 업데이트 함수
	UFUNCTION(BlueprintCallable, Category = "City UI")
	void UpdateProductionInfo();

	// 버튼 클릭 이벤트 핸들러
	UFUNCTION()
	void OnProductionBtnClicked();

	UFUNCTION()
	void OnPurchaseBtnClicked();

	// 생산 시작 델리게이트 핸들러
	UFUNCTION()
	void OnProductionStarted(FName ProductionID);

	// 생산 완료 델리게이트 핸들러
	UFUNCTION()
	void OnProductionCompleted(FName ProductionID);

	// 생산 진행도 변경 델리게이트 핸들러
	UFUNCTION()
	void OnProductionProgressChanged();

	// 시설 변경 델리게이트 핸들러
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);

	// 타일 구매 버튼 클릭 핸들러
	UFUNCTION()
	void OnPurchaseTileBtnClicked();

	// 타일 구매 모드 닫기 버튼 클릭 핸들러
	UFUNCTION()
	void OnCloseBtnClicked();

	// 타일 구매 클릭 핸들러
	UFUNCTION()
	void OnPurchaseTileClickedHandler(FVector2D TileCoordinate);

	// 골드 변경 델리게이트 핸들러
	UFUNCTION()
	void OnGoldChanged(int32 NewGold);

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// 도시 컴포넌트 참조
	UPROPERTY()
	UCityComponent* CachedCityComponent = nullptr;

	// 델리게이트 바인딩
	void BindToProductionDelegates();
	void UnbindFromProductionDelegates();
	void BindToFacilityDelegates();
	void UnbindFromFacilityDelegates();
	void BindToPurchaseDelegates();
	void UnbindFromPurchaseDelegates();

	// 타일 구매 관련 함수
	void FindPurchaseableTiles();
	void HighlightPurchaseableTiles();
	void ClearPurchaseableTileHighlights();
	void ExitPurchaseMode();

	// 구매 모드 상태
	UPROPERTY()
	bool bIsTilePurchaseMode = false;

	// 구매 가능한 타일 좌표 배열
	UPROPERTY()
	TArray<FVector2D> PurchaseableTileCoordinates;
};

