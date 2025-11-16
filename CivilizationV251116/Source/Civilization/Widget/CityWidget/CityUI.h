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
	class UTextBlock* FaithTxt = nullptr;

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

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	// 도시 컴포넌트 참조
	UPROPERTY()
	UCityComponent* CachedCityComponent = nullptr;

	// 델리게이트 바인딩
	void BindToProductionDelegates();
	void UnbindFromProductionDelegates();
};

