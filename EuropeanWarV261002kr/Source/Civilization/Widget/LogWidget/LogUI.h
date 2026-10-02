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
	// ========== 로그 추가 ==========

	// 로그 끝에 줄 넘김 후 ----------Round: N---------- 를 붙입니다.
	UFUNCTION(BlueprintCallable, Category = "Log UI")
	void AppendRoundLine(int32 RoundNumber);

	// 로그 끝에 줄 넘김 후 임의 문장을 붙입니다.
	UFUNCTION(BlueprintCallable, Category = "Log UI")
	void AppendLine(const FString& Text);

protected:
	// ========== BindWidget ==========

	// 게임 로그를 보여주는 텍스트
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UTextBlock* LogTxt = nullptr;

	// 로그를 스크롤하는 박스
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UScrollBox* LogSB = nullptr;
};
