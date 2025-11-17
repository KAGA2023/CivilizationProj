// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../World/WorldTileActor.h"
#include "MainHUD.generated.h"

// 플레이어 0의 도시 타일 클릭 이벤트 델리게이트
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCityTileClicked);

// FOnBuilderTileClicked는 WorldTileActor.h에서 이미 선언되어 있음

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

	// OnBuilderTileClicked 델리게이트 콜백 함수
	UFUNCTION()
	void OnBuilderTileClickedHandler(class UWorldTile* Tile, FVector2D TileCoordinate);

	// OnGoldChanged 델리게이트 콜백 함수
	UFUNCTION()
	void OnGoldChanged(int32 NewGold);

	// OnPopulationChanged 델리게이트 콜백 함수
	UFUNCTION()
	void OnPopulationChanged(int32 NewPopulation);

	// OnFacilityChanged 델리게이트 콜백 함수
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);

	// 모든 WorldTileActor의 OnPlayerCityTileClicked 델리게이트 바인딩
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindCityTileClickedDelegates();

	// 모든 WorldTileActor의 OnBuilderTileClicked 델리게이트 바인딩
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindBuilderTileClickedDelegates();

	// PlayerState의 델리게이트 바인딩
	void BindPlayerStateDelegates();

	// FacilityManager의 델리게이트 바인딩
	void BindFacilityDelegates();
	void UnbindFacilityDelegates();

	// 플레이어 0의 도시 타일 클릭 이벤트
	UPROPERTY(BlueprintAssignable, Category = "City Events")
	FOnCityTileClicked OnCityTileClicked;

	// 건설자 클릭 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Builder Events")
	FOnBuilderTileClicked OnBuilderTileClicked;

	// 0.5초 지연 후 바인딩을 위한 타이머
	FTimerHandle BindCityTileTimerHandle;

	// 0.5초 지연 후 바인딩을 위한 타이머
	FTimerHandle BindBuilderTileTimerHandle;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};

