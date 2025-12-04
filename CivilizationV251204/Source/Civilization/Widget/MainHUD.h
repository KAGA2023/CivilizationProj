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

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UHorizontalBox* StrategicResourceHB = nullptr;

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

	// OnGeneralTileClicked 델리게이트 콜백 함수
	UFUNCTION()
	void OnGeneralTileClickedHandler(FVector2D TileCoordinate);

	// OnStrategicResourceStockChanged 델리게이트 콜백 함수
	UFUNCTION()
	void OnStrategicResourceStockChanged(EStrategicResource Resource, int32 NewStock);

	// 모든 WorldTileActor의 OnPlayerCityTileClicked 델리게이트 바인딩
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindCityTileClickedDelegates();

	// 모든 WorldTileActor의 OnBuilderTileClicked 델리게이트 바인딩
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindBuilderTileClickedDelegates();

	// 모든 WorldTileActor의 OnGeneralTileClicked 델리게이트 바인딩
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindGeneralTileClickedDelegates();

	// PlayerState의 델리게이트 바인딩
	void BindPlayerStateDelegates();

	// FacilityManager의 델리게이트 바인딩
	void BindFacilityDelegates();
	void UnbindFacilityDelegates();

	// 전략 자원 슬롯 관리
	void UpdateStrategicResourceSlots();
	void ClearStrategicResourceSlots();

	// 플레이어 0의 도시 타일 클릭 이벤트
	UPROPERTY(BlueprintAssignable, Category = "City Events")
	FOnCityTileClicked OnCityTileClicked;

	// 건설자 클릭 이벤트
	UPROPERTY(BlueprintAssignable, Category = "Builder Events")
	FOnBuilderTileClicked OnBuilderTileClicked;

	// BuildFacilityUI 위젯 참조 (블루프린트에서 수동 할당)
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class UBuildFacilityUI* BuildFacilityUIWidget = nullptr;

	// 현재 열린 타일 좌표 추적
	FVector2D CurrentOpenFacilityTile = FVector2D::ZeroVector;
	bool bIsFacilityUIOpen = false;

	// UI 닫기 함수
	UFUNCTION()
	void CloseFacilityUI();

	// 0.5초 지연 후 바인딩을 위한 타이머
	FTimerHandle BindCityTileTimerHandle;

	// 0.5초 지연 후 바인딩을 위한 타이머
	FTimerHandle BindBuilderTileTimerHandle;

	// 0.5초 지연 후 바인딩을 위한 타이머
	FTimerHandle BindGeneralTileTimerHandle;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};

