// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "NextRoundButtonUI.generated.h"

/**
 * 다음 턴 버튼 UI 위젯
 */
UCLASS()
class CIVILIZATION_API UNextRoundButtonUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// 다음 턴 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* NextRoundBtn = nullptr;

	// 버튼 클릭 핸들러
	UFUNCTION()
	void OnNextRoundButtonClicked();
};

