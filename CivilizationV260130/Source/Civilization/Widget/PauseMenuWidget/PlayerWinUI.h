// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerWinUI.generated.h"

/**
 * 플레이어 승리 위젯
 */
UCLASS()
class CIVILIZATION_API UPlayerWinUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// 타이틀 메뉴 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* TitleMenuBtn;

private:
	// 타이틀 메뉴 버튼 클릭 이벤트
	UFUNCTION()
	void OnTitleMenuButtonClicked();
};
