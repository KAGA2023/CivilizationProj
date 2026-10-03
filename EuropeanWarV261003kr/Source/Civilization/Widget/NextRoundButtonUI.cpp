// Fill out your copyright notice in the Description page of Project Settings.

#include "NextRoundButtonUI.h"
#include "Components/Button.h"
#include "../SuperGameController.h"

void UNextRoundButtonUI::NativeConstruct()
{
	Super::NativeConstruct();

	// 버튼 클릭 이벤트 바인딩
	if (NextRoundBtn)
	{
		NextRoundBtn->OnClicked.AddDynamic(this, &UNextRoundButtonUI::OnNextRoundButtonClicked);
	}
}

void UNextRoundButtonUI::OnNextRoundButtonClicked()
{
	if (ASuperGameController* GameController = Cast<ASuperGameController>(GetOwningPlayer()))
	{
		GameController->ServerRequestEndPlayerTurn();
	}
}

