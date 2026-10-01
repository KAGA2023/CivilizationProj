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
	virtual void NativeConstruct() override;

	// 방 만들기 성공 시 로비 맵으로 이동합니다. 월드는 아직 생성하지 않습니다.

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* CreateSessionBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidgetOptional))
	class UTextBlock* WarningTxt = nullptr;

	UFUNCTION()
	void OnCreateSessionButtonClicked();

private:
	class UMultiplayerSessionSubsystem* GetSessionSubsystem() const;
	void SetCreateSessionButtonEnabled(bool bEnabled);
	void SetWarningText(const FString& Message);
};
