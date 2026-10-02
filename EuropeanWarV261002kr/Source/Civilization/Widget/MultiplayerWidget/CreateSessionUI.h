// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CreateSessionUI.generated.h"

UCLASS()
class CIVILIZATION_API UCreateSessionUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// ========== 수명 주기 ==========

	// 방 만들기 버튼을 묶고 경고 텍스트를 숨깁니다.
	virtual void NativeConstruct() override;

protected:
	// ========== BindWidget ==========

	// 로비 맵으로 이동해 호스트 세션을 만들게 하는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CreateSessionBtn = nullptr;

	// 로비 이동 실패 등을 보여 주는 경고 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* WarningTxt = nullptr;

	// 세션 서브시스템으로 로비 listen 이동을 요청합니다.
	UFUNCTION()
	void OnCreateSessionButtonClicked();

private:
	// ========== 세션 헬퍼 ==========

	// 멀티플레이 세션 서브시스템을 반환합니다.
	class UMultiplayerSessionSubsystem* GetSessionSubsystem() const;
	// 이동 중에는 방 만들기 버튼을 잠급니다.
	void SetCreateSessionButtonEnabled(bool bEnabled);
	// 경고 텍스트를 보여 주거나 숨깁니다.
	void SetWarningText(const FString& Message);
};
