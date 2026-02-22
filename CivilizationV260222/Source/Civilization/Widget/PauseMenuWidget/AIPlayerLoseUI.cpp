// Fill out your copyright notice in the Description page of Project Settings.

#include "AIPlayerLoseUI.h"
#include "Components/Button.h"

void UAIPlayerLoseUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 계속하기 버튼 클릭 이벤트 바인딩
	if (ContinueBtn)
	{
		ContinueBtn->OnClicked.AddDynamic(this, &UAIPlayerLoseUI::OnContinueButtonClicked);
	}
}

void UAIPlayerLoseUI::OnContinueButtonClicked()
{
	// 계속하기 버튼 클릭 델리게이트 브로드캐스트
	OnContinueButtonClickedDelegate.Broadcast();
}
