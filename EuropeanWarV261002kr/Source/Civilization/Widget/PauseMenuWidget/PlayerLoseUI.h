// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerLoseUI.generated.h"

// 플레이어가 패배했을 때 보여주는 위젯입니다.
UCLASS()
class CIVILIZATION_API UPlayerLoseUI : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

protected:
	// ========== BindWidget ==========

	// 패배한 플레이어 국왕 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* KingImg = nullptr;

	// 패배한 플레이어 국가 국기 이미지
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UImage* CountryImg = nullptr;

	// 타이틀 메뉴로 돌아가는 버튼
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	class UButton* TitleMenuBtn;

private:
	// 타이틀 메뉴 버튼을 누르면 메인 메뉴로 돌아갑니다.
	UFUNCTION()
	void OnTitleMenuButtonClicked();
};
