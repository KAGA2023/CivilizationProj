// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../World/WorldTileActor.h"
#include "../Diplomacy/DiplomacyStruct.h"
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
	class UButton* PauseBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UHorizontalBox* StrategicResourceHB = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UHorizontalBox* CountryHB = nullptr;

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

	// ========== 승리/패배 시스템 델리게이트 핸들러 ==========
	// 플레이어 승리 핸들러
	UFUNCTION()
	void OnPlayerVictory();

	// 플레이어 패배 핸들러
	UFUNCTION()
	void OnPlayerDefeated();

	// AI 플레이어 패배 핸들러
	UFUNCTION()
	void OnAIPlayerDefeated(int32 DefeatedPlayerIndex);

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

	// 국가 슬롯 관리
	void UpdateCountrySlots();
	void ClearCountrySlots();

	// 국가 슬롯 클릭 콜백
	UFUNCTION()
	void OnCountrySlotClicked(int32 TargetPlayerIndex);

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

	// UnitCombatUI 위젯 참조 (블루프린트에서 수동 할당)
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class UUnitCombatUI* UnitCombatUIWidget = nullptr;

	// 현재 열린 전투 타일 좌표 추적
	FVector2D CurrentCombatHoverTile = FVector2D::ZeroVector;
	bool bIsCombatUIOpen = false;

	// DiplomacyUI 위젯 참조 (블루프린트에서 수동 할당)
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class UDiplomacyUI* DiplomacyUIWidget = nullptr;

	// PauseMenuUI 위젯 참조 (블루프린트에서 수동 할당)
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class UPauseMenuUI* PauseMenuUIWidget = nullptr;

	// ========== 승리/패배 시스템 위젯 ==========
	// PlayerWinUI 위젯 참조 (블루프린트에서 수동 할당)
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class UPlayerWinUI* PlayerWinUIWidget = nullptr;

	// PlayerLoseUI 위젯 참조 (블루프린트에서 수동 할당)
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class UPlayerLoseUI* PlayerLoseUIWidget = nullptr;

	// AIPlayerLoseUI 위젯 참조 (블루프린트에서 수동 할당)
	UPROPERTY(BlueprintReadWrite, Category = "Widgets")
	class UAIPlayerLoseUI* AIPlayerLoseUIWidget = nullptr;

	// 현재 외교 대상 플레이어 인덱스
	int32 CurrentDiplomacyTargetPlayer = -1;
	bool bIsDiplomacyUIOpen = false;

	// 외교 UI 열기 함수
	void OpenDiplomacyUI(int32 TargetPlayerIndex);

	// 외교 UI 닫기 함수
	UFUNCTION()
	void CloseDiplomacyUI();

	// 전투 타일 호버 델리게이트 바인딩 함수
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindCombatTileHoverDelegates();

	// UnitManager의 전투 실행 완료 델리게이트 바인딩
	void BindCombatExecutedDelegate();

	// 전투 타일 호버 핸들러 함수들
	UFUNCTION()
	void OnCombatTileHoverBeginHandler(class UWorldTile* Tile);

	UFUNCTION()
	void OnCombatTileHoverEndHandler(class UWorldTile* Tile);

	// 전투 실행 완료 핸들러
	UFUNCTION()
	void OnCombatExecutedHandler();

	// 전투 UI 닫기 함수
	void CloseCombatUI();

	// DiplomacyManager 델리게이트 바인딩 함수
	void BindDiplomacyDelegates();

	// DiplomacyManager 델리게이트 언바인딩 함수
	void UnbindDiplomacyDelegates();

	// PauseMenuUI 위젯 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void SetPauseMenuUI(class UPauseMenuUI* InPauseMenuUI);

	// ========== 승리/패배 시스템 위젯 설정 함수 ==========
	// PlayerWinUI 위젯 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void SetPlayerWinUI(class UPlayerWinUI* InPlayerWinUI);

	// PlayerLoseUI 위젯 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void SetPlayerLoseUI(class UPlayerLoseUI* InPlayerLoseUI);

	// AIPlayerLoseUI 위젯 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Widgets")
	void SetAIPlayerLoseUI(class UAIPlayerLoseUI* InAIPlayerLoseUI);

	// ========== 승리/패배 위젯 표시 함수 ==========
	// 플레이어 승리 위젯 표시
	UFUNCTION(BlueprintCallable, Category = "Victory")
	void ShowVictoryWidget();

	// 플레이어 패배 위젯 표시
	UFUNCTION(BlueprintCallable, Category = "Victory")
	void ShowDefeatWidget();

	// AI 플레이어 패배 위젯 표시
	UFUNCTION(BlueprintCallable, Category = "Victory")
	void ShowAIDefeatWidget(int32 DefeatedPlayerIndex);

	// PauseBtn 클릭 핸들러
	UFUNCTION()
	void OnPauseButtonClicked();

	// PauseMenuUI 델리게이트 바인딩 함수
	void BindPauseMenuUIDelegate();

	// PauseMenuUI 숨기기 함수
	UFUNCTION()
	void HidePauseMenu();

	// AIPlayerLoseUI 델리게이트 바인딩 함수
	void BindAIPlayerLoseUIDelegate();

	// AIPlayerLoseUI 숨기기 함수
	UFUNCTION()
	void HideAIPlayerLoseUI();

	// 외교 액션 발행 핸들러
	UFUNCTION()
	void OnDiplomacyActionIssuedHandler(const struct FDiplomacyAction& Action);

	// 외교 액션 처리 핸들러
	UFUNCTION()
	void OnDiplomacyActionResolvedHandler(const struct FDiplomacyAction& Action, bool bAccepted);

	// 외교 상태 변경 핸들러
	UFUNCTION()
	void OnDiplomacyStatusChangedHandler(int32 PlayerA, int32 PlayerB, EDiplomacyStatusType NewStatus);

	// 0.5초 지연 후 바인딩을 위한 타이머
	FTimerHandle BindCityTileTimerHandle;

	// 0.5초 지연 후 바인딩을 위한 타이머
	FTimerHandle BindBuilderTileTimerHandle;

	// 0.5초 지연 후 바인딩을 위한 타이머
	FTimerHandle BindGeneralTileTimerHandle;

	// 0.5초 지연 후 바인딩을 위한 타이머
	FTimerHandle BindCombatTileHoverTimerHandle;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
};

