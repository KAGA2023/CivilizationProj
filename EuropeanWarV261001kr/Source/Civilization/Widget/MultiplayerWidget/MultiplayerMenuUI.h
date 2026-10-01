// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MultiplayerMenuUI.generated.h"

class UCreateSessionUI;
class UFindSessionUI;

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
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintAssignable, Category = "Menu Events")
	FOnMultiplayerMenuBackButtonClicked OnBackButtonClickedDelegate;

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* BackBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UCreateSessionUI* CreateSessionUIWidget = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UFindSessionUI* FindSessionUIWidget = nullptr;

	UFUNCTION()
	void OnBackButtonClicked();
};
