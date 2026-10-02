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
	// 다음 턴 버튼을 묶습니다.
	virtual void NativeConstruct() override;

protected:
	// ========== BindWidget ==========

	// 로컬 플레이어 턴을 끝내는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* NextRoundBtn = nullptr;

	// 호스트에 내 턴 종료를 요청합니다. 싱글·호스트는 그 자리에서 처리됩니다.
	UFUNCTION()
	void OnNextRoundButtonClicked();
};
