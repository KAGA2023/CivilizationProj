// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerMenuUI.generated.h"

class UCreateSessionUI;
class UFindSessionUI;

// 뒤로가기 시 메인 메뉴 캔버스를 다시 보여 줍니다.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnMultiplayerMenuBackButtonClicked);

/**
 * 메인 메뉴에서 로비에 들어가기 전 화면입니다.
 * 호스트는 방 만들기 후 로비 맵으로, 참가자는 목록에서 Join 후 같은 로비로 갑니다.
 */
UCLASS()
class CIVILIZATION_API UMultiplayerMenuUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 수명 주기 ==========

	// 뒤로가기 버튼을 묶습니다.
	virtual void NativeConstruct() override;

	// 메인 메뉴가 이 화면을 닫을 때 듣는 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnMultiplayerMenuBackButtonClicked OnBackButtonClickedDelegate;

protected:
	// ========== BindWidget ==========

	// 멀티플레이 메뉴를 닫고 메인 메뉴로 돌아가는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn = nullptr;

	// 방 만들기 UI
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCreateSessionUI* CreateSessionUIWidget = nullptr;

	// 로비 찾기 UI
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UFindSessionUI* FindSessionUIWidget = nullptr;

	// 뒤로가기 델리게이트를 브로드캐스트합니다.
	UFUNCTION()
	void OnBackButtonClicked();
};
