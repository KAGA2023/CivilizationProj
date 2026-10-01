// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FindSessionsCallbackProxy.h"
#include "SessionItemUI.generated.h"

UCLASS()
class CIVILIZATION_API USessionItemUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Session")
	void SetSearchResult(const FBlueprintSessionResult& InSearchResult);

	// Join 성공 시 호스트가 열어 둔 로비 맵으로 이동합니다.

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* ServerNameTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* PlayerCountTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* JoinBtn = nullptr;

	UPROPERTY(BlueprintReadWrite, Category = "Session")
	FBlueprintSessionResult SearchResult;

	UFUNCTION()
	void OnJoinButtonClicked();

	UFUNCTION()
	void OnJoinSessionComplete(bool bWasSuccessful);

private:
	class UMultiplayerSessionSubsystem* GetSessionSubsystem() const;
	void RefreshSessionInfo();
};
