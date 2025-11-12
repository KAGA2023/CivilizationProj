// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MainHUD.generated.h"

// 플레이어 0의 도시 타일 클릭 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCityTileClicked);

UCLASS()
class CIVILIZATION_API UMainHUD : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* FaithTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* GoldTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PopulationTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ScienceTxt = nullptr;

	// 데이터 업데이트 함수
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHUDData();

	// OnTurnChanged 델리게이트 콜백 함수
	UFUNCTION()
	void OnTurnChanged(FTurnStruct NewTurn);

	// OnPlayerCityTileClicked 델리게이트 콜백 함수
	UFUNCTION()
	void OnPlayerCityTileClicked();

	// 모든 WorldTileActor의 OnPlayerCityTileClicked 델리게이트 바인딩
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindCityTileClickedDelegates();

	// 플레이어 0의 도시 타일 클릭 이벤트
	UPROPERTY(BlueprintAssignable, Category = "City Events")
	FOnCityTileClicked OnCityTileClicked;

	// 0.1초 지연 후 바인딩을 위한 타이머
	FTimerHandle BindCityTileTimerHandle;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};

