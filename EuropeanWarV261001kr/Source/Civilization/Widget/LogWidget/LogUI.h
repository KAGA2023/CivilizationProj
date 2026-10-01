// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "LogUI.generated.h"

UCLASS()
class CIVILIZATION_API ULogUI : public UUserWidget
{
	GENERATED_BODY()

public:
	// LogTxt 뒤에 줄 넘김 후 "----------Round: N----------" 추가
	UFUNCTION(BlueprintCallable, Category = "Log UI")
	void AppendRoundLine(int32 RoundNumber);

	// LogTxt 뒤에 줄 넘김 후 임의 문장 추가
	UFUNCTION(BlueprintCallable, Category = "Log UI")
	void AppendLine(const FString& Text);

protected:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* LogTxt = nullptr;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UScrollBox* LogSB = nullptr;
};
