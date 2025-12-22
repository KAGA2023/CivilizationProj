// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "DiplomacyUI.generated.h"

class ASuperPlayerState;

UCLASS()
class CIVILIZATION_API UDiplomacyUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// 플레이어 정보 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Diplomacy UI")
	void SetupForPlayer(int32 TargetPlayerIndex, ASuperPlayerState* TargetPlayerState);

	// 대상 플레이어 인덱스 가져오기
	UFUNCTION(BlueprintCallable, Category = "Diplomacy UI")
	int32 GetTargetPlayerIndex() const { return CurrentTargetPlayerIndex; }

	// 버튼 상태 업데이트 함수
	UFUNCTION(BlueprintCallable, Category = "Diplomacy UI")
	void UpdateButtonStates();

private:
	// DiplomacyManager 가져오기 헬퍼 함수
	class UDiplomacyManager* GetDiplomacyManager() const;

	// 현재 라운드 가져오기 헬퍼 함수
	int32 GetCurrentRound() const;

protected:
	virtual void NativeConstruct() override;

	// 버튼 클릭 핸들러
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

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* CountryTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

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
	// 대상 플레이어 인덱스
	UPROPERTY()
	int32 CurrentTargetPlayerIndex = -1;
};

