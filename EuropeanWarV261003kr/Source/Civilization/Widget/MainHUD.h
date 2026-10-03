// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Animation/WidgetAnimation.h"
#include "../World/WorldTileActor.h"
#include "../Diplomacy/DiplomacyStruct.h"
#include "MainHUD.generated.h"

// 내 도시 타일을 클릭했을 때 브로드캐스트합니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnCityTileClicked);

// FOnBuilderTileClicked는 WorldTileActor.h에서 이미 선언되어 있습니다.

UCLASS()
class CIVILIZATION_API UMainHUD : public UUserWidget
{
	GENERATED_BODY()

protected:
	// ========== 상단 자원 / 라운드 ==========

	// 보유 골드를 표시하는 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* GoldTxt = nullptr;

	// 인구를 표시하는 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PopulationTxt = nullptr;

	// 과학량을 표시하는 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ScienceTxt = nullptr;

	// 현재 라운드를 표시하는 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* RoundTxt = nullptr;

	// 일시정지 메뉴를 여는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* PauseBtn = nullptr;

	// ========== 위젯 애니메이션 ==========

	// 연구 UI를 열 때 재생하는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* OpenResearchUI = nullptr;

	// AI 패배 위젯을 띄울 때 재생하는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* OpenAIPlayerLoseUI = nullptr;

	// 플레이어 승리 위젯을 띄울 때 재생하는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* OpenPlayerWinUI = nullptr;

	// 플레이어 패배 위젯을 띄울 때 재생하는 애니메이션
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* OpenPlayerLoseUI = nullptr;

	// ========== 상단 슬롯 박스 ==========

	// 전략 자원 슬롯을 담는 가로 박스
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UHorizontalBox* StrategicResourceHB = nullptr;

	// 사치 자원 슬롯을 담는 가로 박스
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UHorizontalBox* LuxuryResourceHB = nullptr;

	// 국가 슬롯을 담는 가로 박스
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UHorizontalBox* CountryHB = nullptr;

	// ========== HUD 갱신 ==========

	// HUD 숫자와 슬롯을 현재 상태로 갱신합니다.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void UpdateHUDData();

	// 턴이 바뀔 때 HUD를 갱신합니다.
	UFUNCTION()
	void OnTurnChanged(FTurnStruct NewTurn);

	// 라운드가 바뀔 때 로그에 라운드 줄을 추가합니다.
	UFUNCTION()
	void OnRoundChangedHandler(FTurnStruct NewTurn);

	// 내 도시 타일을 클릭하면 도시 UI를 엽니다.
	UFUNCTION()
	void OnPlayerCityTileClicked();

	// 건설자 타일을 클릭하면 시설 건설 UI를 엽니다.
	UFUNCTION()
	void OnBuilderTileClickedHandler(class UWorldTile* Tile, FVector2D TileCoordinate);

	// 골드가 바뀌면 골드 텍스트를 갱신합니다.
	UFUNCTION()
	void OnGoldChanged(int32 NewGold);

	// 인구가 바뀌면 인구 텍스트를 갱신합니다.
	UFUNCTION()
	void OnPopulationChanged(int32 NewPopulation);

	// 시설이 바뀌면 HUD 자원을 다시 그립니다.
	UFUNCTION()
	void OnFacilityChanged(FVector2D TileCoordinate);

	// 일반 타일 클릭을 받아 관련 UI를 닫거나 처리합니다.
	UFUNCTION()
	void OnGeneralTileClickedHandler(FVector2D TileCoordinate);

	// 전략 자원 수량이 바뀌면 슬롯을 갱신합니다.
	UFUNCTION()
	void OnStrategicResourceStockChanged(EStrategicResource Resource, int32 NewStock);

	// 사치 자원이 바뀌면 슬롯을 갱신합니다.
	UFUNCTION()
	void OnLuxuryResourceChanged(ELuxuryResource Resource, int32 NewAmount);

	// ========== 승리 / 패배 핸들러 ==========

	// 플레이어 승리 시 승리 위젯을 표시합니다.
	UFUNCTION()
	void OnPlayerVictory();

	// 플레이어 패배 시 패배 위젯을 표시합니다.
	UFUNCTION()
	void OnPlayerDefeated();

	// AI가 패배하면 AI 패배 위젯을 표시합니다.
	UFUNCTION()
	void OnAIPlayerDefeated(int32 DefeatedPlayerIndex);

	// ========== 델리게이트 바인딩 ==========

	// 모든 타일의 도시 클릭 델리게이트를 연결합니다.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindCityTileClickedDelegates();

	// 모든 타일의 건설자 클릭 델리게이트를 연결합니다.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindBuilderTileClickedDelegates();

	// 모든 타일의 일반 클릭 델리게이트를 연결합니다.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindGeneralTileClickedDelegates();

	// 인게임 GameState의 턴 컴포넌트를 반환합니다.
	class UTurnComponent* GetTurnComponent() const;

	// GameState가 생기면 턴 델리게이트를 묶고, 없으면 타이머로 재시도합니다.
	void BindTurnComponent();

	// 지금 턴이 내 슬롯이면 오버레이를 숨기고, 아니면 보이게 합니다.
	void ApplyOtherPlayerTurnOverlay(int32 CurrentPlayerIndex);

	// 로컬 플레이어 스테이트 델리게이트를 연결합니다.
	void BindPlayerStateDelegates();

	// 시설 매니저 델리게이트를 연결합니다.
	void BindFacilityDelegates();

	// 시설 매니저 델리게이트를 해제합니다.
	void UnbindFacilityDelegates();

	// ========== 슬롯 관리 ==========

	// 전략 자원 슬롯을 다시 만듭니다.
	void UpdateStrategicResourceSlots();

	// 전략 자원 슬롯을 비웁니다.
	void ClearStrategicResourceSlots();

	// 사치 자원 슬롯을 다시 만듭니다.
	void UpdateLuxuryResourceSlots();

	// 사치 자원 슬롯을 비웁니다.
	void ClearLuxuryResourceSlots();

	// 국가 슬롯을 다시 만듭니다.
	void UpdateCountrySlots();

	// 국가 슬롯을 비웁니다.
	void ClearCountrySlots();

	// 국가 슬롯을 클릭하면 외교 UI를 엽니다.
	UFUNCTION()
	void OnCountrySlotClicked(int32 TargetPlayerIndex);

	// ========== 이벤트 ==========

	// 내 도시 타일 클릭을 블루프린트에 알립니다.
	UPROPERTY(BlueprintAssignable, Category = "City Events")
	FOnCityTileClicked OnCityTileClicked;

	// 건설자 타일 클릭을 블루프린트에 알립니다.
	UPROPERTY(BlueprintAssignable, Category = "Builder Events")
	FOnBuilderTileClicked OnBuilderTileClicked;

	// ========== 자식 패널 ==========

	// 시설 건설 패널. W_MainHUD 자식 이름 BuildFacilityPanel로 바인딩합니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBuildFacilityUI* BuildFacilityPanel = nullptr;

	// 지금 열린 시설 UI의 타일 좌표
	FVector2D CurrentOpenFacilityTile = FVector2D::ZeroVector;

	// 시설 건설 UI가 열려 있는지
	bool bIsFacilityUIOpen = false;

	// 시설 건설 UI를 닫습니다.
	UFUNCTION()
	void CloseFacilityUI();

	// 전투 예측 패널. W_MainHUD 자식 이름 UnitCombatPanel로 바인딩합니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UUnitCombatUI* UnitCombatPanel = nullptr;

	// 지금 호버 중인 전투 타일 좌표
	FVector2D CurrentCombatHoverTile = FVector2D::ZeroVector;

	// 전투 UI가 열려 있는지
	bool bIsCombatUIOpen = false;

	// 외교 패널. W_MainHUD 자식 이름 DiplomacyPanel로 바인딩합니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UDiplomacyUI* DiplomacyPanel = nullptr;

	// 일시정지 메뉴 패널. W_MainHUD 자식 이름 PauseMenuPanel로 바인딩합니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UPauseMenuUI* PauseMenuPanel = nullptr;

	// 마우스 호버 정보 패널. W_MainHUD 자식 이름 MousePanel로 바인딩합니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UMouseUI* MousePanel = nullptr;

	// 다른 플레이어 턴 안내 패널. W_MainHUD 자식 이름 OtherPlayerTurnPanel로 바인딩합니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UOtherPlayerTurnUI* OtherPlayerTurnPanel = nullptr;

	// ========== 승리 / 패배 위젯 ==========

	// 플레이어 승리 패널. W_MainHUD 자식 이름 PlayerWinPanel로 바인딩합니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UPlayerWinUI* PlayerWinPanel = nullptr;

	// 플레이어 패배 패널. W_MainHUD 자식 이름 PlayerLosePanel로 바인딩합니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UPlayerLoseUI* PlayerLosePanel = nullptr;

	// AI 패배 패널. W_MainHUD 자식 이름 AIPlayerLosePanel로 바인딩합니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UAIPlayerLoseUI* AIPlayerLosePanel = nullptr;

	// 선택 유닛 정보 패널. Optional이라 기존 UnitInfoUIWidget 블루프린트와도 호환됩니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UUnitInfoUI* UnitInfoPanel = nullptr;

	// 연구 열기 위젯. 블루프린트 이름이 같으면 자동 바인딩됩니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UOpenResearchUI* OpenResearchUIWidget = nullptr;

	// 게임 로그 위젯. 블루프린트 이름이 같으면 자동 바인딩됩니다.
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class ULogUI* LogUIWidget = nullptr;

	// ========== 외교 / 로그 상태 ==========

	// 지금 외교 창에 열린 상대 슬롯
	int32 CurrentDiplomacyTargetPlayer = -1;

	// 외교 UI가 열려 있는지
	bool bIsDiplomacyUIOpen = false;

	// NativeConstruct에서 라운드 줄을 이미 넣었는지. 재실행 시 중복을 막습니다.
	bool bInitialRoundLineAppended = false;

	// 로그에 마지막으로 넣은 라운드 번호. 같은 라운드 중복을 막습니다.
	int32 LastAppendedRoundNumber = -1;

	// GameState 턴 바인딩을 재시도하는 타이머
	FTimerHandle BindTurnComponentTimerHandle;

	// 대상 슬롯의 외교 UI를 엽니다.
	void OpenDiplomacyUI(int32 TargetPlayerIndex);

	// 외교 UI를 닫습니다.
	UFUNCTION()
	void CloseDiplomacyUI();

	// ========== 호버 / 전투 / 선택 ==========

	// 전투 가능 타일 호버 델리게이트를 연결합니다.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindCombatTileHoverDelegates();

	// 일반 타일 호버 델리게이트를 연결합니다.
	UFUNCTION(BlueprintCallable, Category = "HUD")
	void BindTileHoverDelegates();

	// 전투 실행 완료 델리게이트를 연결합니다.
	void BindCombatExecutedDelegate();

	// 선택 유닛 변경 델리게이트를 연결합니다.
	void BindSelectedUnitDelegate();

	// 선택 유닛이 바뀌면 UnitInfoUI를 갱신합니다.
	UFUNCTION()
	void OnSelectedUnitChangedHandler(class AUnitCharacterBase* NewSelectedUnit);

	// UnitInfoUI를 숨깁니다.
	void CloseUnitInfoUI();

	// 전투 타일 호버를 시작하면 전투 UI를 엽니다.
	UFUNCTION()
	void OnCombatTileHoverBeginHandler(class UWorldTile* Tile);

	// 전투 타일 호버가 끝나면 전투 UI를 닫습니다.
	UFUNCTION()
	void OnCombatTileHoverEndHandler(class UWorldTile* Tile);

	// 타일 호버를 시작하면 마우스 정보를 표시합니다.
	UFUNCTION()
	void OnTileHoverBeginHandler(class UWorldTile* Tile);

	// 타일 호버가 끝나면 마우스 정보를 숨깁니다.
	UFUNCTION()
	void OnTileHoverEndHandler(class UWorldTile* Tile);

	// 전투가 끝나면 전투 UI를 닫습니다.
	UFUNCTION()
	void OnCombatExecutedHandler();

	// 전투 UI를 닫습니다.
	void CloseCombatUI();

	// 외교 매니저 델리게이트를 연결합니다.
	void BindDiplomacyDelegates();

	// 외교 매니저 델리게이트를 해제합니다.
	void UnbindDiplomacyDelegates();

	// ========== 승리 / 패배 표시 ==========

	// 플레이어 승리 위젯을 표시합니다.
	UFUNCTION(BlueprintCallable, Category = "Victory")
	void ShowVictoryWidget();

	// 플레이어 패배 위젯을 표시합니다.
	UFUNCTION(BlueprintCallable, Category = "Victory")
	void ShowDefeatWidget();

	// AI 패배 위젯을 표시합니다.
	UFUNCTION(BlueprintCallable, Category = "Victory")
	void ShowAIDefeatWidget(int32 DefeatedPlayerIndex);

	// ========== 버튼 / 메뉴 ==========

	// 일시정지 버튼을 누르면 일시정지 메뉴를 엽니다.
	UFUNCTION()
	void OnPauseButtonClicked();

	// 연구 버튼을 누르면 연구 UI 애니메이션을 재생합니다.
	UFUNCTION()
	void OnResearchButtonClicked();

	// 일시정지 메뉴 닫기 델리게이트를 연결합니다.
	void BindPauseMenuUIDelegate();

	// 일시정지 메뉴를 숨깁니다.
	UFUNCTION()
	void HidePauseMenu();

	// AI 패배 UI 닫기 델리게이트를 연결합니다.
	void BindAIPlayerLoseUIDelegate();

	// AI 패배 UI를 숨깁니다.
	UFUNCTION()
	void HideAIPlayerLoseUI();

	// ========== 외교 액션 ==========

	// 외교 액션이 발행되면 로그와 UI를 갱신합니다.
	UFUNCTION()
	void OnDiplomacyActionIssuedHandler(const struct FDiplomacyAction& Action);

	// 외교 액션이 처리되면 로그와 UI를 갱신합니다.
	UFUNCTION()
	void OnDiplomacyActionResolvedHandler(const struct FDiplomacyAction& Action, bool bAccepted);

	// 외교 상태가 바뀌면 국가 슬롯을 갱신합니다.
	UFUNCTION()
	void OnDiplomacyStatusChangedHandler(int32 PlayerA, int32 PlayerB, EDiplomacyStatusType NewStatus);

	// 개발 중 기술 표시를 한 번 갱신합니다. NativeConstruct에서 호출합니다.
	UFUNCTION()
	void RefreshOpenResearchUI();

	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void NativeDestruct() override;
};
