// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FindSessionsCallbackProxy.h"
#include "FindSessionUI.generated.h"

class USessionItemUI;

UCLASS()
class CIVILIZATION_API UFindSessionUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	// 로비 목록만 보여 줍니다. Join은 호스트의 로비 맵으로 들어갑니다.

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* FindSessionBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UVerticalBox* SessionList = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* SessionMessageTxt = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<USessionItemUI> SessionItemWidgetClass;

	UFUNCTION()
	void OnFindSessionButtonClicked();

	UFUNCTION()
	void OnFindSessionsComplete(bool bWasSuccessful);

private:
	class UMultiplayerSessionSubsystem* GetSessionSubsystem() const;
	void SetFindSessionButtonEnabled(bool bEnabled);
	void SetSessionMessage(const FString& Message);
	TSubclassOf<USessionItemUI> ResolveSessionItemWidgetClass();
	void AddItemRenderer(const FBlueprintSessionResult& SearchResult);
	FString GetSessionResultMessage(int32 SessionCount) const;
};
