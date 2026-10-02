// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiplomacyUI.generated.h"

class ASuperPlayerState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDiplomacyUICloseButtonClicked);

UCLASS()
class CIVILIZATION_API UDiplomacyUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 닫기 버튼. MainHUD가 열려 있음 플래그를 내립니다.
	UPROPERTY(BlueprintAssignable, Category = "Diplomacy Events")
	FOnDiplomacyUICloseButtonClicked OnDiplomacyUICloseButtonClicked;

	// 외교 대상 슬롯과 국가 표시를 채웁니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy UI")
	void SetupForPlayer(int32 TargetPlayerIndex, ASuperPlayerState* TargetPlayerState);

	// 지금 열려 있는 외교 대상 슬롯
	UFUNCTION(BlueprintCallable, Category = "Diplomacy UI")
	int32 GetTargetPlayerIndex() const { return CurrentTargetPlayerIndex; }

	// 나와 대상 사이의 상태·쿨다운으로 버튼을 켜고 끕니다.
	UFUNCTION(BlueprintCallable, Category = "Diplomacy UI")
	void UpdateButtonStates();

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnSendGiftBtnClicked();

	UFUNCTION()
	void OnOfferAllianceBtnClicked();

	UFUNCTION()
	void OnOfferPeaceBtnClicked();

	UFUNCTION()
	void OnDenounceBtnClicked();

	UFUNCTION()
	void OnDeclareWarBtnClicked();

	UFUNCTION()
	void OnCloseBtnClicked();

	// ========== BindWidget ==========

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* CountryTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* KingImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* LoseImg = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UBorder* DefeatBrd = nullptr;

	// 나와 대상의 외교 상태 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* StateWithMeTxt = nullptr;

	// 나와 대상의 호감도 숫자
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* AttitudeWithMeTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UHorizontalBox* GoodHB = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UHorizontalBox* BadHB = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UHorizontalBox* WarHB = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UHorizontalBox* AllyHB = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* SendGiftBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* OfferAllianceBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* OfferPeaceBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* DenounceBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* DeclareWarBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CloseBtn = nullptr;

private:
	class UDiplomacyManager* GetDiplomacyManager() const;

	// 호스트 GameMode 턴이 있으면 그 라운드, 없으면 1
	int32 GetCurrentRound() const;

	// 이 기기의 문명 슬롯. 외교 FromPlayerId에 씁니다.
	int32 GetLocalPlayerIndex() const;

	// 대상 국가의 전쟁/동맹/우호/적대 국기를 채웁니다.
	void UpdateRelationBoxes();

	// 외교 창에 열려 있는 상대 슬롯
	UPROPERTY()
	int32 CurrentTargetPlayerIndex = -1;
};
